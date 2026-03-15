-- Supabase/PostgreSQL schema for the transport JSON dataset
-- Target: public schema
-- Run this in Supabase SQL Editor.

begin;

-- =========================
-- 1) Core tables
-- =========================

create table if not exists public.icons_fontawesome (
    icon_name text primary key,
    created_at timestamptz not null default now()
);

create table if not exists public.stops (
    id integer primary key,
    name text not null,
    lat double precision not null,
    lng double precision not null,
    color text,
    icon text,
    icon_type text,
    created_at timestamptz not null default now(),
    updated_at timestamptz not null default now(),
    constraint stops_color_hex_chk check (
        color is null or color ~* '^#[0-9a-f]{6}$'
    )
);

create table if not exists public.routes (
    id integer primary key,
    name text not null,
    color text,
    style text,
    weight integer,
    snap_to_road boolean,
    rating_average numeric(5,2),
    rating_count integer,
    operator text[],
    is_verified_route boolean,
    details jsonb,
    legacy_stops jsonb,
    created_at timestamptz not null default now(),
    updated_at timestamptz not null default now(),
    constraint routes_color_hex_chk check (
        color is null or color ~* '^#[0-9a-f]{6}$'
    ),
    constraint routes_rating_count_chk check (
        rating_count is null or rating_count >= 0
    ),
    constraint routes_weight_chk check (
        weight is null or weight >= 0
    )
);

create table if not exists public.route_stops (
    route_id integer not null references public.routes(id) on delete cascade,
    stop_order integer not null,
    stop_id integer not null references public.stops(id) on delete restrict,
    created_at timestamptz not null default now(),
    primary key (route_id, stop_order)
);

create table if not exists public.vehicles (
    id integer primary key,
    name text not null,
    lat double precision not null,
    lng double precision not null,
    route_id integer references public.routes(id) on delete set null,
    speed integer,
    passengers integer not null default 0,
    color text,
    icon text,
    icon_type text,
    vehicle_image text,
    moving boolean,
    bearing integer,
    rating_average numeric(5,2),
    rating_count integer,
    created_at timestamptz not null default now(),
    updated_at timestamptz not null default now(),
    constraint vehicles_speed_chk check (speed is null or speed >= 0),
    constraint vehicles_passengers_chk check (passengers >= 0),
    constraint vehicles_bearing_chk check (bearing is null or (bearing >= 0 and bearing <= 360)),
    constraint vehicles_color_hex_chk check (
        color is null or color ~* '^#[0-9a-f]{6}$'
    ),
    constraint vehicles_rating_count_chk check (
        rating_count is null or rating_count >= 0
    )
);

create table if not exists public.suggestions (
    id integer primary key,
    name text not null,
    category text,
    message text,
    task jsonb,
    status text,
    created_at timestamptz,
    resolved_at timestamptz,
    inserted_at timestamptz not null default now(),
    constraint suggestions_status_chk check (
        status is null or status in ('pending', 'approved', 'completed', 'rejected')
    )
);

-- Obstructions are currently [] in source JSON.
-- Keep raw JSON payload to remain forward-compatible with unknown shape.
create table if not exists public.obstructions_raw (
    id bigserial primary key,
    payload jsonb not null,
    inserted_at timestamptz not null default now()
);

-- =========================
-- 2) Indexes
-- =========================

create index if not exists idx_stops_name on public.stops(name);
create index if not exists idx_routes_name on public.routes(name);
create index if not exists idx_route_stops_stop_id on public.route_stops(stop_id);
create index if not exists idx_vehicles_route_id on public.vehicles(route_id);
create index if not exists idx_vehicles_name on public.vehicles(name);
create index if not exists idx_suggestions_status on public.suggestions(status);
create index if not exists idx_suggestions_task_gin on public.suggestions using gin(task);
create index if not exists idx_obstructions_payload_gin on public.obstructions_raw using gin(payload);

-- =========================
-- 3) Updated-at trigger
-- =========================

create or replace function public.set_updated_at()
returns trigger
language plpgsql
as $$
begin
    new.updated_at = now();
    return new;
end;
$$;

drop trigger if exists trg_stops_set_updated_at on public.stops;
create trigger trg_stops_set_updated_at
before update on public.stops
for each row
execute function public.set_updated_at();

drop trigger if exists trg_routes_set_updated_at on public.routes;
create trigger trg_routes_set_updated_at
before update on public.routes
for each row
execute function public.set_updated_at();

drop trigger if exists trg_vehicles_set_updated_at on public.vehicles;
create trigger trg_vehicles_set_updated_at
before update on public.vehicles
for each row
execute function public.set_updated_at();

-- =========================
-- 4) JSON sync functions
-- =========================

create or replace function public.sync_icons(p_payload jsonb)
returns void
language plpgsql
as $$
begin
    truncate table public.icons_fontawesome;

    insert into public.icons_fontawesome(icon_name)
    select value
    from jsonb_array_elements_text(coalesce(p_payload -> 'fontawesome', '[]'::jsonb)) as t(value)
    on conflict (icon_name) do nothing;
end;
$$;

create or replace function public.sync_stops(p_payload jsonb)
returns void
language sql
as $$
    insert into public.stops (id, name, lat, lng, color, icon, icon_type)
    select
        s.id,
        s.name,
        s.lat,
        s.lng,
        s.color,
        s.icon,
        s."iconType" as icon_type
    from jsonb_to_recordset(p_payload) as s(
        id integer,
        name text,
        lat double precision,
        lng double precision,
        color text,
        icon text,
        "iconType" text
    )
    on conflict (id)
    do update set
        name = excluded.name,
        lat = excluded.lat,
        lng = excluded.lng,
        color = excluded.color,
        icon = excluded.icon,
        icon_type = excluded.icon_type;
$$;

create or replace function public.sync_routes(p_payload jsonb)
returns void
language plpgsql
as $$
begin
    insert into public.routes (
        id,
        name,
        color,
        style,
        weight,
        snap_to_road,
        rating_average,
        rating_count,
        operator,
        is_verified_route,
        details,
        legacy_stops
    )
    select
        r.id,
        r.name,
        r.color,
        r.style,
        r.weight,
        r."snapToRoad" as snap_to_road,
        r."ratingAverage" as rating_average,
        r."ratingCount" as rating_count,
        case
            when r.operator is null then null
            else (
                select array_agg(op.value)
                from jsonb_array_elements_text(r.operator) as op(value)
            )
        end as operator,
        r."isVerifiedRoute" as is_verified_route,
        r.details,
        r.stops as legacy_stops
    from jsonb_to_recordset(p_payload) as r(
        id integer,
        name text,
        color text,
        style text,
        weight integer,
        "snapToRoad" boolean,
        "ratingAverage" numeric,
        "ratingCount" integer,
        operator jsonb,
        "isVerifiedRoute" boolean,
        details jsonb,
        stops jsonb,
        "stopIds" jsonb
    )
    on conflict (id)
    do update set
        name = excluded.name,
        color = excluded.color,
        style = excluded.style,
        weight = excluded.weight,
        snap_to_road = excluded.snap_to_road,
        rating_average = excluded.rating_average,
        rating_count = excluded.rating_count,
        operator = excluded.operator,
        is_verified_route = excluded.is_verified_route,
        details = excluded.details,
        legacy_stops = excluded.legacy_stops;

    delete from public.route_stops;

    insert into public.route_stops (route_id, stop_order, stop_id)
    select
        (obj ->> 'id')::integer as route_id,
        sid.ordinality::integer as stop_order,
        sid.value::integer as stop_id
    from jsonb_array_elements(p_payload) as r(obj)
    cross join lateral jsonb_array_elements_text(coalesce(r.obj -> 'stopIds', '[]'::jsonb)) with ordinality as sid(value, ordinality)
    where exists (
        select 1
        from public.routes pr
        where pr.id = (obj ->> 'id')::integer
    )
    and exists (
        select 1
        from public.stops ps
        where ps.id = sid.value::integer
    )
    on conflict (route_id, stop_order)
    do update set stop_id = excluded.stop_id;
end;
$$;

create or replace function public.sync_vehicles(p_payload jsonb)
returns void
language sql
as $$
    insert into public.vehicles (
        id,
        name,
        lat,
        lng,
        route_id,
        speed,
        passengers,
        color,
        icon,
        icon_type,
        vehicle_image,
        moving,
        bearing,
        rating_average,
        rating_count
    )
    select
        v.id,
        v.name,
        v.lat,
        v.lng,
        v."routeId" as route_id,
        v.speed,
        coalesce(v.passengers, 0) as passengers,
        v.color,
        v.icon,
        v."iconType" as icon_type,
        v.vehicle_image,
        v.moving,
        v.bearing,
        v."ratingAverage" as rating_average,
        v."ratingCount" as rating_count
    from jsonb_to_recordset(p_payload) as v(
        id integer,
        name text,
        lat double precision,
        lng double precision,
        "routeId" integer,
        speed integer,
        passengers integer,
        color text,
        icon text,
        "iconType" text,
        vehicle_image text,
        moving boolean,
        bearing integer,
        "ratingAverage" numeric,
        "ratingCount" integer
    )
    on conflict (id)
    do update set
        name = excluded.name,
        lat = excluded.lat,
        lng = excluded.lng,
        route_id = excluded.route_id,
        speed = excluded.speed,
        passengers = excluded.passengers,
        color = excluded.color,
        icon = excluded.icon,
        icon_type = excluded.icon_type,
        vehicle_image = excluded.vehicle_image,
        moving = excluded.moving,
        bearing = excluded.bearing,
        rating_average = excluded.rating_average,
        rating_count = excluded.rating_count;
$$;

create or replace function public.sync_suggestions(p_payload jsonb)
returns void
language sql
as $$
    insert into public.suggestions (
        id,
        name,
        category,
        message,
        task,
        status,
        created_at,
        resolved_at
    )
    select
        s.id,
        s.name,
        s.category,
        s.message,
        s.task,
        s.status,
        s.created_at,
        s.resolved_at
    from jsonb_to_recordset(p_payload) as s(
        id integer,
        name text,
        category text,
        message text,
        task jsonb,
        status text,
        created_at timestamptz,
        resolved_at timestamptz
    )
    on conflict (id)
    do update set
        name = excluded.name,
        category = excluded.category,
        message = excluded.message,
        task = excluded.task,
        status = excluded.status,
        created_at = excluded.created_at,
        resolved_at = excluded.resolved_at;
$$;

create or replace function public.sync_obstructions(p_payload jsonb)
returns void
language plpgsql
as $$
begin
    delete from public.obstructions_raw;

    insert into public.obstructions_raw(payload)
    select elem
    from jsonb_array_elements(coalesce(p_payload, '[]'::jsonb)) as t(elem);
end;
$$;

-- =========================
-- 5) Convenience view
-- =========================

create or replace view public.v_routes_with_stops as
select
    r.id,
    r.name,
    r.color,
    r.style,
    r.weight,
    r.snap_to_road,
    r.rating_average,
    r.rating_count,
    r.operator,
    r.is_verified_route,
    r.details,
    jsonb_agg(
        jsonb_build_object(
            'order', rs.stop_order,
            'id', s.id,
            'name', s.name,
            'lat', s.lat,
            'lng', s.lng
        )
        order by rs.stop_order
    ) as stops
from public.routes r
left join public.route_stops rs on rs.route_id = r.id
left join public.stops s on s.id = rs.stop_id
group by
    r.id,
    r.name,
    r.color,
    r.style,
    r.weight,
    r.snap_to_road,
    r.rating_average,
    r.rating_count,
    r.operator,
    r.is_verified_route,
    r.details;

commit;

-- =========================
-- 6) How to load your JSON
-- =========================
-- Replace each JSON literal below with the actual file content and run.
-- Order matters: stops first, then routes, then vehicles.
--
-- select public.sync_icons(
-- $json$
-- {"fontawesome":["fa-bus","fa-shuttle-van","fa-location-dot","fa-route","fa-bus-simple"]}
-- $json$::jsonb
-- );
--
-- select public.sync_stops(
-- $json$
-- [ ... paste stops.json here ... ]
-- $json$::jsonb
-- );
--
-- select public.sync_routes(
-- $json$
-- [ ... paste routes.json here ... ]
-- $json$::jsonb
-- );
--
-- select public.sync_vehicles(
-- $json$
-- [ ... paste vehicles.json here ... ]
-- $json$::jsonb
-- );
--
-- select public.sync_suggestions(
-- $json$
-- [ ... paste suggestions.json here ... ]
-- $json$::jsonb
-- );
--
-- select public.sync_obstructions(
-- $json$
-- [ ... paste obstructions.json here ... ]
-- $json$::jsonb
-- );
