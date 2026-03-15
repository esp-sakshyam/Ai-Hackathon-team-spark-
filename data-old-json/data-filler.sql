-- Data loader for Supabase transport dataset
-- Requires schema/functions from supabase_schema.sql to be created first.

begin;

select public.sync_icons(
$json$
{
  "fontawesome": [
    "fa-bus",
    "fa-shuttle-van",
    "fa-location-dot",
    "fa-route",
    "fa-bus-simple"
  ]
}

$json$::jsonb
);

select public.sync_stops(
$json$
[
    {
        "name": "Teaching Hospital",
        "lat": 27.7362630953893,
        "lng": 85.3327524662018,
        "color": "#3da329",
        "icon": "fa-bus-simple",
        "iconType": "fontawesome",
        "id": 1
    },
    {
        "name": "Panipokhari",
        "lat": 27.729710599956533,
        "lng": 85.32484531402588,
        "color": "#1d4ed8",
        "icon": "fa-bus",
        "iconType": "fontawesome",
        "id": 2
    },
    {
        "name": "Lazimpat",
        "lat": 27.725204343694273,
        "lng": 85.32258152961732,
        "color": "#1d4ed8",
        "icon": "fa-bus",
        "iconType": "fontawesome",
        "id": 3
    },
    {
        "name": "Lainchaur",
        "lat": 27.717886622430626,
        "lng": 85.3167235851288,
        "color": "#1d4ed8",
        "icon": "fa-bus",
        "iconType": "fontawesome",
        "id": 4
    },
    {
        "name": "Jamal",
        "lat": 27.708920438595676,
        "lng": 85.3156077861786,
        "color": "#1d4ed8",
        "icon": "fa-bus",
        "iconType": "fontawesome",
        "id": 5
    },
    {
        "name": "Ghantaghar",
        "lat": 27.707690380251986,
        "lng": 85.31670212745668,
        "color": "#1d4ed8",
        "icon": "fa-bus",
        "iconType": "fontawesome",
        "id": 6
    },
    {
        "name": "Ratnapark",
        "lat": 27.70659328948767,
        "lng": 85.31536102294922,
        "color": "#1d4ed8",
        "icon": "fa-bus",
        "iconType": "fontawesome",
        "id": 7
    },
    {
        "name": "RNAC",
        "lat": 27.701962590039766,
        "lng": 85.31354784965517,
        "color": "#1d4ed8",
        "icon": "fa-bus",
        "iconType": "fontawesome",
        "id": 8
    },
    {
        "name": "Bhadrakali",
        "lat": 27.7028887456512,
        "lng": 85.316401720047,
        "color": "#1d4ed8",
        "icon": "fa-bus",
        "iconType": "fontawesome",
        "id": 9
    },
    {
        "name": "Sahid Gate",
        "lat": 27.699587796144364,
        "lng": 85.314958691597,
        "color": "#1d4ed8",
        "icon": "fa-bus",
        "iconType": "fontawesome",
        "id": 10
    },
    {
        "name": "Bagbazar",
        "lat": 27.70567903875681,
        "lng": 85.32032042741777,
        "color": "#1d4ed8",
        "icon": "fa-bus",
        "iconType": "fontawesome",
        "id": 11
    },
    {
        "name": "Putalisadak Star Mall",
        "lat": 27.7066764028106,
        "lng": 85.32281488180162,
        "color": "#1d4ed8",
        "icon": "fa-bus",
        "iconType": "fontawesome",
        "id": 12
    },
    {
        "name": "Hattisar",
        "lat": 27.708219924443725,
        "lng": 85.32184123992921,
        "color": "#1d4ed8",
        "icon": "fa-bus",
        "iconType": "fontawesome",
        "id": 13
    },
    {
        "name": "Chakrapath",
        "lat": 27.739909530896256,
        "lng": 85.33699035644531,
        "color": "#1d4ed8",
        "icon": "fa-bus",
        "iconType": "fontawesome",
        "id": 14
    },
    {
        "name": "Basundhara Chauki",
        "lat": 27.73829998651819,
        "lng": 85.32559633255006,
        "color": "#1d4ed8",
        "icon": "fa-bus",
        "iconType": "fontawesome",
        "id": 16
    },
    {
        "name": "Samakhusi",
        "lat": 27.736168134500968,
        "lng": 85.32200217247009,
        "color": "#1d4ed8",
        "icon": "fa-bus",
        "iconType": "fontawesome",
        "id": 17
    },
    {
        "name": "Gangalal Hospital",
        "lat": 27.745981904582997,
        "lng": 85.34266591072083,
        "color": "#1d4ed8",
        "icon": "fa-bus",
        "iconType": "fontawesome",
        "id": 18
    },
    {
        "name": "Neuro Hospital Bansbari",
        "lat": 27.748669023671095,
        "lng": 85.34540712833406,
        "color": "#1d4ed8",
        "icon": "fa-bus",
        "iconType": "fontawesome",
        "id": 19
    },
    {
        "name": "Golfutar",
        "lat": 27.750829115503702,
        "lng": 85.34558951854706,
        "color": "#1d4ed8",
        "icon": "fa-bus",
        "iconType": "fontawesome",
        "id": 20
    },
    {
        "name": "Telecom Chowk Hattigauda",
        "lat": 27.75553369469828,
        "lng": 85.34862041473389,
        "color": "#1d4ed8",
        "icon": "fa-bus",
        "iconType": "fontawesome",
        "id": 21
    },
    {
        "name": "Hattigauda",
        "lat": 27.7569056275423,
        "lng": 85.34940361976625,
        "color": "#1d4ed8",
        "icon": "fa-bus",
        "iconType": "fontawesome",
        "id": 22
    },
    {
        "name": "Ghumti",
        "lat": 27.766390005185553,
        "lng": 85.35375952720642,
        "color": "#1d4ed8",
        "icon": "fa-bus",
        "iconType": "fontawesome",
        "id": 23
    },
    {
        "name": "Chapali",
        "lat": 27.76757668095013,
        "lng": 85.35571217536928,
        "color": "#1d4ed8",
        "icon": "fa-bus",
        "iconType": "fontawesome",
        "id": 24
    },
    {
        "name": "Deuba Chowk",
        "lat": 27.771582802742206,
        "lng": 85.35880208015443,
        "color": "#1d4ed8",
        "icon": "fa-bus",
        "iconType": "fontawesome",
        "id": 25
    },
    {
        "name": "Rudreshwor",
        "lat": 27.77680383012511,
        "lng": 85.36201000213624,
        "color": "#1d4ed8",
        "icon": "fa-bus",
        "iconType": "fontawesome",
        "id": 26
    },
    {
        "name": "ASCOL Stop",
        "lat": 27.717321508668093,
        "lng": 85.31478703515569,
        "color": "#1d4ed8",
        "icon": "fa-bus",
        "iconType": "fontawesome",
        "id": 27
    },
    {
        "name": "Adarsha",
        "lat": 27.66651579794753,
        "lng": 85.4340110062304,
        "color": "#1d4ed8",
        "icon": "fa-bus",
        "iconType": "fontawesome",
        "id": 28
    },
    {
        "name": "Airport Bus Station",
        "lat": 27.70038098305323,
        "lng": 85.35388290882112,
        "color": "#1d4ed8",
        "icon": "fa-bus",
        "iconType": "fontawesome",
        "id": 29
    },
    {
        "name": "Anamnagar Stop",
        "lat": 27.699412059342507,
        "lng": 85.32856822754208,
        "color": "#1d4ed8",
        "icon": "fa-bus",
        "iconType": "fontawesome",
        "id": 30
    },
    {
        "name": "Apex College Stop",
        "lat": 27.699022587583922,
        "lng": 85.33827782911163,
        "color": "#1d4ed8",
        "icon": "fa-bus",
        "iconType": "fontawesome",
        "id": 31
    },
    {
        "name": "Attarkhel",
        "lat": 27.73453004615077,
        "lng": 85.38256642611412,
        "color": "#1d4ed8",
        "icon": "fa-bus",
        "iconType": "fontawesome",
        "id": 32
    },
    {
        "name": "B & B Hospital\/KCM Stop",
        "lat": 27.664178242570465,
        "lng": 85.33015608664145,
        "color": "#1d4ed8",
        "icon": "fa-bus",
        "iconType": "fontawesome",
        "id": 33
    },
    {
        "name": "Baakha Bazaar",
        "lat": 27.6828794,
        "lng": 85.3858308,
        "color": "#1d4ed8",
        "icon": "fa-bus",
        "iconType": "fontawesome",
        "id": 34
    },
    {
        "name": "Babarmahal Stop",
        "lat": 27.6900786037129,
        "lng": 85.32892759789512,
        "color": "#1d4ed8",
        "icon": "fa-bus",
        "iconType": "fontawesome",
        "id": 35
    },
    {
        "name": "Babarmahal Stop",
        "lat": 27.692453604506564,
        "lng": 85.32396564941628,
        "color": "#1d4ed8",
        "icon": "fa-bus",
        "iconType": "fontawesome",
        "id": 36
    },
    {
        "name": "Badegaun",
        "lat": 27.611284220843213,
        "lng": 85.3576578505017,
        "color": "#1d4ed8",
        "icon": "fa-bus",
        "iconType": "fontawesome",
        "id": 37
    },
    {
        "name": "Bagbazar Stop",
        "lat": 27.705980623610888,
        "lng": 85.31688995488884,
        "color": "#1d4ed8",
        "icon": "fa-bus",
        "iconType": "fontawesome",
        "id": 38
    },
    {
        "name": "Bal Mandir",
        "lat": 27.714581377724762,
        "lng": 85.32951771842808,
        "color": "#1d4ed8",
        "icon": "fa-bus",
        "iconType": "fontawesome",
        "id": 39
    },
    {
        "name": "Balaju",
        "lat": 27.727027761712318,
        "lng": 85.30458391329698,
        "color": "#1d4ed8",
        "icon": "fa-bus",
        "iconType": "fontawesome",
        "id": 40
    },
    {
        "name": "Balkhu Stop",
        "lat": 27.684777,
        "lng": 85.29797,
        "color": "#1d4ed8",
        "icon": "fa-bus",
        "iconType": "fontawesome",
        "id": 41
    },
    {
        "name": "Balkumari Stop",
        "lat": 27.6736699,
        "lng": 85.3424814,
        "color": "#1d4ed8",
        "icon": "fa-bus",
        "iconType": "fontawesome",
        "id": 42
    },
    {
        "name": "Banasthali Stop",
        "lat": 27.72477213962459,
        "lng": 85.29753355147126,
        "color": "#1d4ed8",
        "icon": "fa-bus",
        "iconType": "fontawesome",
        "id": 43
    },
    {
        "name": "Banepa",
        "lat": 27.630092719333195,
        "lng": 85.52397251375933,
        "color": "#1d4ed8",
        "icon": "fa-bus",
        "iconType": "fontawesome",
        "id": 44
    },
    {
        "name": "Baneshwor Bus Station",
        "lat": 27.688359070894293,
        "lng": 85.33488770200445,
        "color": "#1d4ed8",
        "icon": "fa-bus",
        "iconType": "fontawesome",
        "id": 45
    },
    {
        "name": "Baphal",
        "lat": 27.699116789507865,
        "lng": 85.28153671366024,
        "color": "#1d4ed8",
        "icon": "fa-bus",
        "iconType": "fontawesome",
        "id": 46
    },
    {
        "name": "Barahi Movies",
        "lat": 27.665969422611063,
        "lng": 85.42169990993376,
        "color": "#1d4ed8",
        "icon": "fa-bus",
        "iconType": "fontawesome",
        "id": 47
    },
    {
        "name": "Basundhara",
        "lat": 27.7406158,
        "lng": 85.3293507,
        "color": "#1d4ed8",
        "icon": "fa-bus",
        "iconType": "fontawesome",
        "id": 48
    },
    {
        "name": "Batuk Bhairav",
        "lat": 27.6644749,
        "lng": 85.3240029,
        "color": "#1d4ed8",
        "icon": "fa-bus",
        "iconType": "fontawesome",
        "id": 49
    },
    {
        "name": "Bhadrakali",
        "lat": 27.6991375,
        "lng": 85.318307,
        "color": "#1d4ed8",
        "icon": "fa-bus",
        "iconType": "fontawesome",
        "id": 50
    },
    {
        "name": "Bhaktapur Bus Park",
        "lat": 27.6713069,
        "lng": 85.4224157,
        "color": "#1d4ed8",
        "icon": "fa-bus",
        "iconType": "fontawesome",
        "id": 51
    },
    {
        "name": "Bhimsengola stop",
        "lat": 27.7003049893476,
        "lng": 85.34327744944058,
        "color": "#1d4ed8",
        "icon": "fa-bus",
        "iconType": "fontawesome",
        "id": 52
    },
    {
        "name": "Bhrikuti Mandap",
        "lat": 27.700527308560346,
        "lng": 85.31656616982016,
        "color": "#1d4ed8",
        "icon": "fa-bus",
        "iconType": "fontawesome",
        "id": 53
    },
    {
        "name": "Bhrikuti Mandap Stop",
        "lat": 27.7020670797842,
        "lng": 85.31876205904446,
        "color": "#1d4ed8",
        "icon": "fa-bus",
        "iconType": "fontawesome",
        "id": 54
    },
    {
        "name": "Bijulibazar Stop",
        "lat": 27.692258856386132,
        "lng": 85.3296196460724,
        "color": "#1d4ed8",
        "icon": "fa-bus",
        "iconType": "fontawesome",
        "id": 55
    },
    {
        "name": "Bir Hospital",
        "lat": 27.7057227,
        "lng": 85.3140743,
        "color": "#1d4ed8",
        "icon": "fa-bus",
        "iconType": "fontawesome",
        "id": 56
    },
    {
        "name": "BishnuDevi Health Centre",
        "lat": 27.680562,
        "lng": 85.2764393,
        "color": "#1d4ed8",
        "icon": "fa-bus",
        "iconType": "fontawesome",
        "id": 57
    },
    {
        "name": "Boudha",
        "lat": 27.720479459796827,
        "lng": 85.36311507225038,
        "color": "#1d4ed8",
        "icon": "fa-bus",
        "iconType": "fontawesome",
        "id": 58
    },
    {
        "name": "Buddhanagar Stop",
        "lat": 27.689266341585014,
        "lng": 85.33101440092973,
        "color": "#1d4ed8",
        "icon": "fa-bus",
        "iconType": "fontawesome",
        "id": 59
    },
    {
        "name": "Byasi Stop",
        "lat": 27.67478236099022,
        "lng": 85.42736472550186,
        "color": "#1d4ed8",
        "icon": "fa-bus",
        "iconType": "fontawesome",
        "id": 60
    },
    {
        "name": "Chabahil \u091a\u093e\u092c\u093e\u0939\u0940\u0932",
        "lat": 27.71700808296587,
        "lng": 85.34660870861605,
        "color": "#1d4ed8",
        "icon": "fa-bus",
        "iconType": "fontawesome",
        "id": 61
    },
    {
        "name": "Changu Narayan",
        "lat": 27.7163215,
        "lng": 85.43078,
        "color": "#1d4ed8",
        "icon": "fa-bus",
        "iconType": "fontawesome",
        "id": 62
    },
    {
        "name": "Chapgaun",
        "lat": 27.5929413,
        "lng": 85.3244031,
        "color": "#1d4ed8",
        "icon": "fa-bus",
        "iconType": "fontawesome",
        "id": 63
    },
    {
        "name": "Chardobato",
        "lat": 27.673504410217177,
        "lng": 85.37998106536044,
        "color": "#1d4ed8",
        "icon": "fa-bus",
        "iconType": "fontawesome",
        "id": 64
    },
    {
        "name": "Chobhar Gate",
        "lat": 27.6704494,
        "lng": 85.2906221,
        "color": "#1d4ed8",
        "icon": "fa-bus",
        "iconType": "fontawesome",
        "id": 65
    },
    {
        "name": "Chunudevi",
        "lat": 27.667114432136746,
        "lng": 85.4168612135083,
        "color": "#1d4ed8",
        "icon": "fa-bus",
        "iconType": "fontawesome",
        "id": 66
    },
    {
        "name": "Chyamasingh",
        "lat": 27.67341889624895,
        "lng": 85.43833494926754,
        "color": "#1d4ed8",
        "icon": "fa-bus",
        "iconType": "fontawesome",
        "id": 67
    },
    {
        "name": "Ciy Center Stop",
        "lat": 27.7094618498415,
        "lng": 85.32783329486847,
        "color": "#1d4ed8",
        "icon": "fa-bus",
        "iconType": "fontawesome",
        "id": 68
    },
    {
        "name": "Daanchi",
        "lat": 27.726959997524837,
        "lng": 85.41472571664072,
        "color": "#1d4ed8",
        "icon": "fa-bus",
        "iconType": "fontawesome",
        "id": 69
    },
    {
        "name": "Damkal",
        "lat": 27.67579901003647,
        "lng": 85.31569899239035,
        "color": "#1d4ed8",
        "icon": "fa-bus",
        "iconType": "fontawesome",
        "id": 70
    },
    {
        "name": "Deuba Chowk stop",
        "lat": 27.7737519327256,
        "lng": 85.3603738447658,
        "color": "#1d4ed8",
        "icon": "fa-bus",
        "iconType": "fontawesome",
        "id": 71
    },
    {
        "name": "Dhokashi",
        "lat": 27.670661,
        "lng": 85.2790338,
        "color": "#1d4ed8",
        "icon": "fa-bus",
        "iconType": "fontawesome",
        "id": 72
    },
    {
        "name": "Dholahiti",
        "lat": 27.647910814381905,
        "lng": 85.31981774230151,
        "color": "#1d4ed8",
        "icon": "fa-bus",
        "iconType": "fontawesome",
        "id": 73
    },
    {
        "name": "Dhulikhel Stop",
        "lat": 27.6184151,
        "lng": 85.5540152,
        "color": "#1d4ed8",
        "icon": "fa-bus",
        "iconType": "fontawesome",
        "id": 74
    },
    {
        "name": "Dhumbarahi",
        "lat": 27.7317994,
        "lng": 85.3443707,
        "color": "#1d4ed8",
        "icon": "fa-bus",
        "iconType": "fontawesome",
        "id": 75
    },
    {
        "name": "Dhungeadda Pipalbot",
        "lat": 27.691308137313797,
        "lng": 85.27559835539985,
        "color": "#1d4ed8",
        "icon": "fa-bus",
        "iconType": "fontawesome",
        "id": 76
    },
    {
        "name": "Dhungedhara",
        "lat": 27.7234101,
        "lng": 85.2946749,
        "color": "#1d4ed8",
        "icon": "fa-bus",
        "iconType": "fontawesome",
        "id": 77
    },
    {
        "name": "Dillibazaar stop",
        "lat": 27.705392,
        "lng": 85.326787,
        "color": "#1d4ed8",
        "icon": "fa-bus",
        "iconType": "fontawesome",
        "id": 78
    },
    {
        "name": "Gahabahal",
        "lat": 27.674297,
        "lng": 85.3210465,
        "color": "#1d4ed8",
        "icon": "fa-bus",
        "iconType": "fontawesome",
        "id": 79
    },
    {
        "name": "Gairidhara",
        "lat": 27.71772041280447,
        "lng": 85.32786012436215,
        "color": "#1d4ed8",
        "icon": "fa-bus",
        "iconType": "fontawesome",
        "id": 80
    },
    {
        "name": "Gairigaun Stop",
        "lat": 27.687499294329665,
        "lng": 85.35062672928755,
        "color": "#1d4ed8",
        "icon": "fa-bus",
        "iconType": "fontawesome",
        "id": 81
    },
    {
        "name": "Gaushala Chowk Stop",
        "lat": 27.707776,
        "lng": 85.343315,
        "color": "#1d4ed8",
        "icon": "fa-bus",
        "iconType": "fontawesome",
        "id": 82
    },
    {
        "name": "Godawari",
        "lat": 27.594516419374894,
        "lng": 85.37770364655141,
        "color": "#1d4ed8",
        "icon": "fa-bus",
        "iconType": "fontawesome",
        "id": 83
    },
    {
        "name": "Gokarna Bus Stop",
        "lat": 27.7403061,
        "lng": 85.3907196,
        "color": "#1d4ed8",
        "icon": "fa-bus",
        "iconType": "fontawesome",
        "id": 84
    },
    {
        "name": "Gongabu Stop",
        "lat": 27.734868893125114,
        "lng": 85.31333058741495,
        "color": "#1d4ed8",
        "icon": "fa-bus",
        "iconType": "fontawesome",
        "id": 85
    },
    {
        "name": "Gopi Krishna Stop",
        "lat": 27.7226748,
        "lng": 85.3453352,
        "color": "#1d4ed8",
        "icon": "fa-bus",
        "iconType": "fontawesome",
        "id": 86
    },
    {
        "name": "Gopi Krishna Stop",
        "lat": 27.7206294,
        "lng": 85.3460992,
        "color": "#1d4ed8",
        "icon": "fa-bus",
        "iconType": "fontawesome",
        "id": 87
    },
    {
        "name": "Gothatar",
        "lat": 27.703133,
        "lng": 85.3694428,
        "color": "#1d4ed8",
        "icon": "fa-bus",
        "iconType": "fontawesome",
        "id": 88
    },
    {
        "name": "Gwarko Chowk Stop",
        "lat": 27.66663,
        "lng": 85.332157,
        "color": "#1d4ed8",
        "icon": "fa-bus",
        "iconType": "fontawesome",
        "id": 89
    },
    {
        "name": "Gyaneshwor Chowk",
        "lat": 27.70863548422915,
        "lng": 85.33180296914617,
        "color": "#1d4ed8",
        "icon": "fa-bus",
        "iconType": "fontawesome",
        "id": 90
    },
    {
        "name": "Hanumanthan Stop",
        "lat": 27.6935216,
        "lng": 85.3276028,
        "color": "#1d4ed8",
        "icon": "fa-bus",
        "iconType": "fontawesome",
        "id": 91
    },
    {
        "name": "Harihar bhawan",
        "lat": 27.681488883201045,
        "lng": 85.31774777191163,
        "color": "#1d4ed8",
        "icon": "fa-bus",
        "iconType": "fontawesome",
        "id": 92
    },
    {
        "name": "Harisiddhi",
        "lat": 27.637705892764977,
        "lng": 85.34055601757453,
        "color": "#1d4ed8",
        "icon": "fa-bus",
        "iconType": "fontawesome",
        "id": 93
    },
    {
        "name": "Hattiban",
        "lat": 27.65009545101879,
        "lng": 85.33242349909301,
        "color": "#1d4ed8",
        "icon": "fa-bus",
        "iconType": "fontawesome",
        "id": 94
    },
    {
        "name": "Hyatt Regency",
        "lat": 27.719330261098836,
        "lng": 85.35410285119426,
        "color": "#1d4ed8",
        "icon": "fa-bus",
        "iconType": "fontawesome",
        "id": 95
    },
    {
        "name": "Indrayeni Stop",
        "lat": 27.729979968401523,
        "lng": 85.4337127377398,
        "color": "#1d4ed8",
        "icon": "fa-bus",
        "iconType": "fontawesome",
        "id": 96
    },
    {
        "name": "Jadibuti",
        "lat": 27.6751899,
        "lng": 85.3539203,
        "color": "#1d4ed8",
        "icon": "fa-bus",
        "iconType": "fontawesome",
        "id": 97
    },
    {
        "name": "Jadibuti",
        "lat": 27.675371448506223,
        "lng": 85.35150112942641,
        "color": "#1d4ed8",
        "icon": "fa-bus",
        "iconType": "fontawesome",
        "id": 98
    },
    {
        "name": "Jagati",
        "lat": 27.66648729147576,
        "lng": 85.43646812315572,
        "color": "#1d4ed8",
        "icon": "fa-bus",
        "iconType": "fontawesome",
        "id": 99
    },
    {
        "name": "Jawalakhel",
        "lat": 27.67310534445946,
        "lng": 85.31353711835128,
        "color": "#1d4ed8",
        "icon": "fa-bus",
        "iconType": "fontawesome",
        "id": 100
    },
    {
        "name": "JayaBageshwari Stop",
        "lat": 27.712064385694443,
        "lng": 85.34499406567794,
        "color": "#1d4ed8",
        "icon": "fa-bus",
        "iconType": "fontawesome",
        "id": 101
    },
    {
        "name": "Jorpati",
        "lat": 27.721747363315263,
        "lng": 85.37267446394551,
        "color": "#1d4ed8",
        "icon": "fa-bus",
        "iconType": "fontawesome",
        "id": 102
    },
    {
        "name": "Jwagal",
        "lat": 27.68549945635466,
        "lng": 85.31810761731965,
        "color": "#1d4ed8",
        "icon": "fa-bus",
        "iconType": "fontawesome",
        "id": 103
    },
    {
        "name": "KMC stop",
        "lat": 27.696514736143723,
        "lng": 85.35147429006139,
        "color": "#1d4ed8",
        "icon": "fa-bus",
        "iconType": "fontawesome",
        "id": 104
    },
    {
        "name": "Kalanki",
        "lat": 27.693342,
        "lng": 85.281683,
        "color": "#1d4ed8",
        "icon": "fa-bus",
        "iconType": "fontawesome",
        "id": 105
    },
    {
        "name": "Kalimati",
        "lat": 27.698339,
        "lng": 85.299423,
        "color": "#1d4ed8",
        "icon": "fa-bus",
        "iconType": "fontawesome",
        "id": 106
    },
    {
        "name": "Kamal Binayak",
        "lat": 27.677632754953567,
        "lng": 85.43758913351822,
        "color": "#1d4ed8",
        "icon": "fa-bus",
        "iconType": "fontawesome",
        "id": 107
    },
    {
        "name": "Kamal Pokhari Mahalaxmi",
        "lat": 27.650803,
        "lng": 85.355846,
        "color": "#1d4ed8",
        "icon": "fa-bus",
        "iconType": "fontawesome",
        "id": 108
    },
    {
        "name": "Kantipath",
        "lat": 27.711532485605012,
        "lng": 85.31500688004938,
        "color": "#1d4ed8",
        "icon": "fa-bus",
        "iconType": "fontawesome",
        "id": 109
    },
    {
        "name": "Kapan Stop",
        "lat": 27.7294183,
        "lng": 85.3540637,
        "color": "#1d4ed8",
        "icon": "fa-bus",
        "iconType": "fontawesome",
        "id": 110
    },
    {
        "name": "KattyaniChowk Stop",
        "lat": 27.695370034798852,
        "lng": 85.34442544970595,
        "color": "#1d4ed8",
        "icon": "fa-bus",
        "iconType": "fontawesome",
        "id": 111
    },
    {
        "name": "Khasibazaar",
        "lat": 27.68929959220008,
        "lng": 85.28431172203615,
        "color": "#1d4ed8",
        "icon": "fa-bus",
        "iconType": "fontawesome",
        "id": 112
    },
    {
        "name": "Khumaltar",
        "lat": 27.656999,
        "lng": 85.326272,
        "color": "#1d4ed8",
        "icon": "fa-bus",
        "iconType": "fontawesome",
        "id": 113
    },
    {
        "name": "Khumaltar",
        "lat": 27.65393750576522,
        "lng": 85.32164489915544,
        "color": "#1d4ed8",
        "icon": "fa-bus",
        "iconType": "fontawesome",
        "id": 114
    },
    {
        "name": "Koteshwor Stop",
        "lat": 27.679043672445687,
        "lng": 85.3495967227744,
        "color": "#1d4ed8",
        "icon": "fa-bus",
        "iconType": "fontawesome",
        "id": 115
    },
    {
        "name": "Krishna Mandir Imadol",
        "lat": 27.659669,
        "lng": 85.343878,
        "color": "#1d4ed8",
        "icon": "fa-bus",
        "iconType": "fontawesome",
        "id": 116
    },
    {
        "name": "Krishna Pauroti Stop",
        "lat": 27.710288209194037,
        "lng": 85.32253855504507,
        "color": "#1d4ed8",
        "icon": "fa-bus",
        "iconType": "fontawesome",
        "id": 117
    },
    {
        "name": "Kumari Club",
        "lat": 27.682644517304933,
        "lng": 85.2985274952127,
        "color": "#1d4ed8",
        "icon": "fa-bus",
        "iconType": "fontawesome",
        "id": 118
    },
    {
        "name": "Kumaripati",
        "lat": 27.6707964353916,
        "lng": 85.32007635133306,
        "color": "#1d4ed8",
        "icon": "fa-bus",
        "iconType": "fontawesome",
        "id": 119
    },
    {
        "name": "Kupondole stop",
        "lat": 27.68793630867814,
        "lng": 85.31623007594492,
        "color": "#1d4ed8",
        "icon": "fa-bus",
        "iconType": "fontawesome",
        "id": 120
    },
    {
        "name": "Lagankhel",
        "lat": 27.662577,
        "lng": 85.323966,
        "color": "#1d4ed8",
        "icon": "fa-bus",
        "iconType": "fontawesome",
        "id": 121
    },
    {
        "name": "Lagankhel Stop",
        "lat": 27.667100178979855,
        "lng": 85.32250107815388,
        "color": "#1d4ed8",
        "icon": "fa-bus",
        "iconType": "fontawesome",
        "id": 122
    },
    {
        "name": "Lamatar",
        "lat": 27.636850301951274,
        "lng": 85.38833163929476,
        "color": "#1d4ed8",
        "icon": "fa-bus",
        "iconType": "fontawesome",
        "id": 123
    },
    {
        "name": "Lazimpat stop",
        "lat": 27.7225227,
        "lng": 85.3209237,
        "color": "#1d4ed8",
        "icon": "fa-bus",
        "iconType": "fontawesome",
        "id": 124
    },
    {
        "name": "Lokanthali",
        "lat": 27.67478236099022,
        "lng": 85.3598320385448,
        "color": "#1d4ed8",
        "icon": "fa-bus",
        "iconType": "fontawesome",
        "id": 125
    },
    {
        "name": "Lubhu",
        "lat": 27.64285727889514,
        "lng": 85.37340784858537,
        "color": "#1d4ed8",
        "icon": "fa-bus",
        "iconType": "fontawesome",
        "id": 126
    },
    {
        "name": "Mahakali Stop",
        "lat": 27.67594153017469,
        "lng": 85.4317849985549,
        "color": "#1d4ed8",
        "icon": "fa-bus",
        "iconType": "fontawesome",
        "id": 127
    },
    {
        "name": "Mahalaxmi Stop",
        "lat": 27.67608880078887,
        "lng": 85.43393608082816,
        "color": "#1d4ed8",
        "icon": "fa-bus",
        "iconType": "fontawesome",
        "id": 128
    },
    {
        "name": "Mahapal",
        "lat": 27.6735497,
        "lng": 85.3238901,
        "color": "#1d4ed8",
        "icon": "fa-bus",
        "iconType": "fontawesome",
        "id": 129
    },
    {
        "name": "Maiti Devi chowk",
        "lat": 27.70389563411532,
        "lng": 85.33266663921509,
        "color": "#1d4ed8",
        "icon": "fa-bus",
        "iconType": "fontawesome",
        "id": 130
    },
    {
        "name": "Maitighar",
        "lat": 27.693784,
        "lng": 85.321208,
        "color": "#1d4ed8",
        "icon": "fa-bus",
        "iconType": "fontawesome",
        "id": 131
    },
    {
        "name": "Maitighar Stop-South",
        "lat": 27.694035326370795,
        "lng": 85.31930923215133,
        "color": "#1d4ed8",
        "icon": "fa-bus",
        "iconType": "fontawesome",
        "id": 132
    },
    {
        "name": "Matatirtha Stop",
        "lat": 27.680611337173485,
        "lng": 85.23284018409879,
        "color": "#1d4ed8",
        "icon": "fa-bus",
        "iconType": "fontawesome",
        "id": 133
    },
    {
        "name": "Milan Chowk Stop",
        "lat": 27.69513729322283,
        "lng": 85.34067033981023,
        "color": "#1d4ed8",
        "icon": "fa-bus",
        "iconType": "fontawesome",
        "id": 134
    },
    {
        "name": "Minbhawan",
        "lat": 27.687565796625947,
        "lng": 85.33849775297604,
        "color": "#1d4ed8",
        "icon": "fa-bus",
        "iconType": "fontawesome",
        "id": 135
    },
    {
        "name": "Mulpani tiwari tole",
        "lat": 27.718906332825792,
        "lng": 85.3937491077006,
        "color": "#1d4ed8",
        "icon": "fa-bus",
        "iconType": "fontawesome",
        "id": 136
    },
    {
        "name": "Nagaon Dobato",
        "lat": 27.6747945,
        "lng": 85.2777568,
        "color": "#1d4ed8",
        "icon": "fa-bus",
        "iconType": "fontawesome",
        "id": 137
    },
    {
        "name": "Naikap",
        "lat": 27.687213765087346,
        "lng": 85.26638764364287,
        "color": "#1d4ed8",
        "icon": "fa-bus",
        "iconType": "fontawesome",
        "id": 138
    },
    {
        "name": "Narayantar",
        "lat": 27.721834570761054,
        "lng": 85.38283701641284,
        "color": "#1d4ed8",
        "icon": "fa-bus",
        "iconType": "fontawesome",
        "id": 139
    },
    {
        "name": "Natole",
        "lat": 27.67601279017407,
        "lng": 85.31813980629566,
        "color": "#1d4ed8",
        "icon": "fa-bus",
        "iconType": "fontawesome",
        "id": 140
    },
    {
        "name": "Naxal Bhagwati",
        "lat": 27.712648522982875,
        "lng": 85.32867015142386,
        "color": "#1d4ed8",
        "icon": "fa-bus",
        "iconType": "fontawesome",
        "id": 141
    },
    {
        "name": "Naya Bato",
        "lat": 27.6761142,
        "lng": 85.2938967,
        "color": "#1d4ed8",
        "icon": "fa-bus",
        "iconType": "fontawesome",
        "id": 142
    },
    {
        "name": "Naya Bazar",
        "lat": 27.6771493,
        "lng": 85.2813895,
        "color": "#1d4ed8",
        "icon": "fa-bus",
        "iconType": "fontawesome",
        "id": 143
    },
    {
        "name": "Naya Buspark",
        "lat": 27.73508730564873,
        "lng": 85.30823924275693,
        "color": "#1d4ed8",
        "icon": "fa-bus",
        "iconType": "fontawesome",
        "id": 144
    },
    {
        "name": "Naya Naikap",
        "lat": 27.701121918909866,
        "lng": 85.28910757838634,
        "color": "#1d4ed8",
        "icon": "fa-bus",
        "iconType": "fontawesome",
        "id": 145
    },
    {
        "name": "New Baneshwor",
        "lat": 27.690364,
        "lng": 85.335923,
        "color": "#1d4ed8",
        "icon": "fa-bus",
        "iconType": "fontawesome",
        "id": 146
    },
    {
        "name": "New Plaza Stop",
        "lat": 27.70077044996401,
        "lng": 85.32330027279126,
        "color": "#1d4ed8",
        "icon": "fa-bus",
        "iconType": "fontawesome",
        "id": 147
    },
    {
        "name": "Nikosera",
        "lat": 27.6800127769328,
        "lng": 85.39840222392165,
        "color": "#1d4ed8",
        "icon": "fa-bus",
        "iconType": "fontawesome",
        "id": 148
    },
    {
        "name": "Panauti",
        "lat": 27.5871361,
        "lng": 85.5133345,
        "color": "#1d4ed8",
        "icon": "fa-bus",
        "iconType": "fontawesome",
        "id": 149
    },
    {
        "name": "Panga Dobato",
        "lat": 27.6747581,
        "lng": 85.2804077,
        "color": "#1d4ed8",
        "icon": "fa-bus",
        "iconType": "fontawesome",
        "id": 150
    },
    {
        "name": "Patan Dhoka",
        "lat": 27.6797052,
        "lng": 85.3204857,
        "color": "#1d4ed8",
        "icon": "fa-bus",
        "iconType": "fontawesome",
        "id": 151
    },
    {
        "name": "Patan Hospital",
        "lat": 27.668477975548733,
        "lng": 85.32170711467148,
        "color": "#1d4ed8",
        "icon": "fa-bus",
        "iconType": "fontawesome",
        "id": 152
    },
    {
        "name": "Pepsi Cola Chowk",
        "lat": 27.6892092,
        "lng": 85.3603535,
        "color": "#1d4ed8",
        "icon": "fa-bus",
        "iconType": "fontawesome",
        "id": 153
    },
    {
        "name": "Pingalasthan",
        "lat": 27.705728907176034,
        "lng": 85.3470647433766,
        "color": "#1d4ed8",
        "icon": "fa-bus",
        "iconType": "fontawesome",
        "id": 154
    },
    {
        "name": "Police Headquarter Stop",
        "lat": 27.716623422918367,
        "lng": 85.33024727557836,
        "color": "#1d4ed8",
        "icon": "fa-bus",
        "iconType": "fontawesome",
        "id": 155
    },
    {
        "name": "Pyanggau Stop",
        "lat": 27.597588152597943,
        "lng": 85.32381904532127,
        "color": "#1d4ed8",
        "icon": "fa-bus",
        "iconType": "fontawesome",
        "id": 156
    },
    {
        "name": "Radhe Radhe",
        "lat": 27.674611334987365,
        "lng": 85.39737227046693,
        "color": "#1d4ed8",
        "icon": "fa-bus",
        "iconType": "fontawesome",
        "id": 157
    },
    {
        "name": "Rata Makai Chowk",
        "lat": 27.661726060074432,
        "lng": 85.34232103035316,
        "color": "#1d4ed8",
        "icon": "fa-bus",
        "iconType": "fontawesome",
        "id": 158
    },
    {
        "name": "Ratna Rajya Stop",
        "lat": 27.695697772095034,
        "lng": 85.33754827319528,
        "color": "#1d4ed8",
        "icon": "fa-bus",
        "iconType": "fontawesome",
        "id": 159
    },
    {
        "name": "Ratopul Stop",
        "lat": 27.708079821073568,
        "lng": 85.3362020819012,
        "color": "#1d4ed8",
        "icon": "fa-bus",
        "iconType": "fontawesome",
        "id": 160
    },
    {
        "name": "Ravi Bhavan",
        "lat": 27.695316403835115,
        "lng": 85.28934050577658,
        "color": "#1d4ed8",
        "icon": "fa-bus",
        "iconType": "fontawesome",
        "id": 161
    },
    {
        "name": "Sallaghari Stop",
        "lat": 27.669798743537886,
        "lng": 85.41116414233265,
        "color": "#1d4ed8",
        "icon": "fa-bus",
        "iconType": "fontawesome",
        "id": 162
    },
    {
        "name": "Sallaghari Stop",
        "lat": 27.674045997128292,
        "lng": 85.4097533620867,
        "color": "#1d4ed8",
        "icon": "fa-bus",
        "iconType": "fontawesome",
        "id": 163
    },
    {
        "name": "Sallaghari Stop",
        "lat": 27.670929460283837,
        "lng": 85.40944755324145,
        "color": "#1d4ed8",
        "icon": "fa-bus",
        "iconType": "fontawesome",
        "id": 164
    },
    {
        "name": "Sanagaun Imadole",
        "lat": 27.649795,
        "lng": 85.358958,
        "color": "#1d4ed8",
        "icon": "fa-bus",
        "iconType": "fontawesome",
        "id": 165
    },
    {
        "name": "Sanepa height",
        "lat": 27.6744676,
        "lng": 85.3022166,
        "color": "#1d4ed8",
        "icon": "fa-bus",
        "iconType": "fontawesome",
        "id": 166
    },
    {
        "name": "Sanga",
        "lat": 27.64186917834339,
        "lng": 85.47739869812064,
        "color": "#1d4ed8",
        "icon": "fa-bus",
        "iconType": "fontawesome",
        "id": 167
    },
    {
        "name": "Sangam Chowk Stop",
        "lat": 27.693194594179232,
        "lng": 85.34054695572772,
        "color": "#1d4ed8",
        "icon": "fa-bus",
        "iconType": "fontawesome",
        "id": 168
    },
    {
        "name": "Sankhamul Stop",
        "lat": 27.6811284,
        "lng": 85.3315875,
        "color": "#1d4ed8",
        "icon": "fa-bus",
        "iconType": "fontawesome",
        "id": 169
    },
    {
        "name": "Sankhu Stop",
        "lat": 27.731025878831137,
        "lng": 85.46559064787976,
        "color": "#1d4ed8",
        "icon": "fa-bus",
        "iconType": "fontawesome",
        "id": 170
    },
    {
        "name": "Sankhu old buspark",
        "lat": 27.727708518431992,
        "lng": 85.46173796247722,
        "color": "#1d4ed8",
        "icon": "fa-bus",
        "iconType": "fontawesome",
        "id": 171
    },
    {
        "name": "SanoThimi",
        "lat": 27.6826801,
        "lng": 85.3723428,
        "color": "#1d4ed8",
        "icon": "fa-bus",
        "iconType": "fontawesome",
        "id": 172
    },
    {
        "name": "Sataungal",
        "lat": 27.686848519721273,
        "lng": 85.25122926103484,
        "color": "#1d4ed8",
        "icon": "fa-bus",
        "iconType": "fontawesome",
        "id": 173
    },
    {
        "name": "Satdobato Chowk Stop",
        "lat": 27.65889,
        "lng": 85.324588,
        "color": "#1d4ed8",
        "icon": "fa-bus",
        "iconType": "fontawesome",
        "id": 174
    },
    {
        "name": "Saughal",
        "lat": 27.670479,
        "lng": 85.3272192,
        "color": "#1d4ed8",
        "icon": "fa-bus",
        "iconType": "fontawesome",
        "id": 175
    },
    {
        "name": "SchEMS",
        "lat": 27.69433931833766,
        "lng": 85.3371459183986,
        "color": "#1d4ed8",
        "icon": "fa-bus",
        "iconType": "fontawesome",
        "id": 176
    },
    {
        "name": "Shantinagar Stop",
        "lat": 27.687048027677736,
        "lng": 85.34099229990366,
        "color": "#1d4ed8",
        "icon": "fa-bus",
        "iconType": "fontawesome",
        "id": 177
    },
    {
        "name": "ShivaDarshan Hall Stop",
        "lat": 27.692301606003294,
        "lng": 85.33618570114437,
        "color": "#1d4ed8",
        "icon": "fa-bus",
        "iconType": "fontawesome",
        "id": 178
    },
    {
        "name": "Shivapuri Park Stop",
        "lat": 27.7910669,
        "lng": 85.3704099,
        "color": "#1d4ed8",
        "icon": "fa-bus",
        "iconType": "fontawesome",
        "id": 179
    },
    {
        "name": "Shobhabhagwati",
        "lat": 27.71811931548166,
        "lng": 85.31207264950397,
        "color": "#1d4ed8",
        "icon": "fa-bus",
        "iconType": "fontawesome",
        "id": 180
    },
    {
        "name": "Sinamangal",
        "lat": 27.698405129508508,
        "lng": 85.34848094973647,
        "color": "#1d4ed8",
        "icon": "fa-bus",
        "iconType": "fontawesome",
        "id": 181
    },
    {
        "name": "Sinamangal\/Ring Road Stop \u0938\u093f\u0928\u093e\u092e\u0902\u0917\u0932",
        "lat": 27.695265538642442,
        "lng": 85.35500944030912,
        "color": "#1d4ed8",
        "icon": "fa-bus",
        "iconType": "fontawesome",
        "id": 182
    },
    {
        "name": "Singha Durbar East Stop",
        "lat": 27.6966083,
        "lng": 85.3281501,
        "color": "#1d4ed8",
        "icon": "fa-bus",
        "iconType": "fontawesome",
        "id": 183
    },
    {
        "name": "Singha Durbar West Stop",
        "lat": 27.696626,
        "lng": 85.3211991,
        "color": "#1d4ed8",
        "icon": "fa-bus",
        "iconType": "fontawesome",
        "id": 184
    },
    {
        "name": "Sitapaila",
        "lat": 27.7073860292121,
        "lng": 85.28248625165054,
        "color": "#1d4ed8",
        "icon": "fa-bus",
        "iconType": "fontawesome",
        "id": 185
    },
    {
        "name": "Soltimode",
        "lat": 27.696575855305454,
        "lng": 85.29355825521812,
        "color": "#1d4ed8",
        "icon": "fa-bus",
        "iconType": "fontawesome",
        "id": 186
    },
    {
        "name": "Sorhakhutte",
        "lat": 27.718860130867945,
        "lng": 85.30961036928913,
        "color": "#1d4ed8",
        "icon": "fa-bus",
        "iconType": "fontawesome",
        "id": 187
    },
    {
        "name": "Srijananagar",
        "lat": 27.67293906663023,
        "lng": 85.40416898564284,
        "color": "#1d4ed8",
        "icon": "fa-bus",
        "iconType": "fontawesome",
        "id": 188
    },
    {
        "name": "Sundhara Stop",
        "lat": 27.7005417,
        "lng": 85.3135033,
        "color": "#1d4ed8",
        "icon": "fa-bus",
        "iconType": "fontawesome",
        "id": 189
    },
    {
        "name": "Suryabinayek",
        "lat": 27.66574612060028,
        "lng": 85.42421580348098,
        "color": "#1d4ed8",
        "icon": "fa-bus",
        "iconType": "fontawesome",
        "id": 190
    },
    {
        "name": "Swayambhu",
        "lat": 27.716029567194173,
        "lng": 85.28353768658415,
        "color": "#1d4ed8",
        "icon": "fa-bus",
        "iconType": "fontawesome",
        "id": 191
    },
    {
        "name": "TU Gate",
        "lat": 27.6788796,
        "lng": 85.2972123,
        "color": "#1d4ed8",
        "icon": "fa-bus",
        "iconType": "fontawesome",
        "id": 192
    },
    {
        "name": "Teaching Hospital stop",
        "lat": 27.7345232,
        "lng": 85.3307108,
        "color": "#1d4ed8",
        "icon": "fa-bus",
        "iconType": "fontawesome",
        "id": 193
    },
    {
        "name": "Teku",
        "lat": 27.696609244453143,
        "lng": 85.3056282805329,
        "color": "#1d4ed8",
        "icon": "fa-bus",
        "iconType": "fontawesome",
        "id": 194
    },
    {
        "name": "Tempo Stop Near Old Baneshwar",
        "lat": 27.701497134508173,
        "lng": 85.34012310500754,
        "color": "#1d4ed8",
        "icon": "fa-bus",
        "iconType": "fontawesome",
        "id": 195
    },
    {
        "name": "Thaiba",
        "lat": 27.62504965657903,
        "lng": 85.34774441348245,
        "color": "#1d4ed8",
        "icon": "fa-bus",
        "iconType": "fontawesome",
        "id": 196
    },
    {
        "name": "Thali",
        "lat": 27.726697546646307,
        "lng": 85.40489200798392,
        "color": "#1d4ed8",
        "icon": "fa-bus",
        "iconType": "fontawesome",
        "id": 197
    },
    {
        "name": "Thankot",
        "lat": 27.706621785491237,
        "lng": 85.20568550866945,
        "color": "#1d4ed8",
        "icon": "fa-bus",
        "iconType": "fontawesome",
        "id": 198
    },
    {
        "name": "Thankot Checkpost",
        "lat": 27.688924334670688,
        "lng": 85.22722363471986,
        "color": "#1d4ed8",
        "icon": "fa-bus",
        "iconType": "fontawesome",
        "id": 199
    },
    {
        "name": "Thapagaun Stop",
        "lat": 27.691080860349,
        "lng": 85.33396483948351,
        "color": "#1d4ed8",
        "icon": "fa-bus",
        "iconType": "fontawesome",
        "id": 200
    },
    {
        "name": "Thapathali",
        "lat": 27.690758,
        "lng": 85.317475,
        "color": "#1d4ed8",
        "icon": "fa-bus",
        "iconType": "fontawesome",
        "id": 201
    },
    {
        "name": "Thecho",
        "lat": 27.61775422523509,
        "lng": 85.31952219972803,
        "color": "#1d4ed8",
        "icon": "fa-bus",
        "iconType": "fontawesome",
        "id": 202
    },
    {
        "name": "Thimi Stop",
        "lat": 27.67335713611914,
        "lng": 85.38628935937297,
        "color": "#1d4ed8",
        "icon": "fa-bus",
        "iconType": "fontawesome",
        "id": 203
    },
    {
        "name": "Tikabhairab",
        "lat": 27.575893584585273,
        "lng": 85.31370172066798,
        "color": "#1d4ed8",
        "icon": "fa-bus",
        "iconType": "fontawesome",
        "id": 204
    },
    {
        "name": "Tinkune",
        "lat": 27.676654,
        "lng": 85.2835434,
        "color": "#1d4ed8",
        "icon": "fa-bus",
        "iconType": "fontawesome",
        "id": 205
    },
    {
        "name": "Tinkune Bhaktapur",
        "lat": 27.67222644449583,
        "lng": 85.40683508906447,
        "color": "#1d4ed8",
        "icon": "fa-bus",
        "iconType": "fontawesome",
        "id": 206
    },
    {
        "name": "Tinkune Subidhanagar",
        "lat": 27.686097986524956,
        "lng": 85.3454661813534,
        "color": "#1d4ed8",
        "icon": "fa-bus",
        "iconType": "fontawesome",
        "id": 207
    },
    {
        "name": "Tribhuvan park",
        "lat": 27.690362660766993,
        "lng": 85.22172356062526,
        "color": "#1d4ed8",
        "icon": "fa-bus",
        "iconType": "fontawesome",
        "id": 208
    },
    {
        "name": "Tripureshwor",
        "lat": 27.693765,
        "lng": 85.314111,
        "color": "#1d4ed8",
        "icon": "fa-bus",
        "iconType": "fontawesome",
        "id": 209
    },
    {
        "name": "Tyogal",
        "lat": 27.6686312,
        "lng": 85.3298476,
        "color": "#1d4ed8",
        "icon": "fa-bus",
        "iconType": "fontawesome",
        "id": 210
    },
    {
        "name": "UN Stop",
        "lat": 27.67807455936041,
        "lng": 85.3163480610597,
        "color": "#1d4ed8",
        "icon": "fa-bus",
        "iconType": "fontawesome",
        "id": 211
    },
    {
        "name": "gathaghar",
        "lat": 27.673979486599617,
        "lng": 85.37304461125746,
        "color": "#1d4ed8",
        "icon": "fa-bus",
        "iconType": "fontawesome",
        "id": 212
    },
    {
        "name": "kaushaltar",
        "lat": 27.67451156969539,
        "lng": 85.36425770316207,
        "color": "#1d4ed8",
        "icon": "fa-bus",
        "iconType": "fontawesome",
        "id": 213
    },
    {
        "name": "lele",
        "lat": 27.57190406505785,
        "lng": 85.33188030025386,
        "color": "#1d4ed8",
        "icon": "fa-bus",
        "iconType": "fontawesome",
        "id": 214
    },
    {
        "name": "saat tale chowk",
        "lat": 27.7269595,
        "lng": 85.350484,
        "color": "#1d4ed8",
        "icon": "fa-bus",
        "iconType": "fontawesome",
        "id": 215
    },
    {
        "name": "Nepal Yatayat Bus Station",
        "lat": 27.717765655722005,
        "lng": 85.40395828201305,
        "color": "#1d4ed8",
        "icon": "fa-bus",
        "iconType": "fontawesome",
        "id": 216
    },
    {
        "name": "Milan Chwok",
        "lat": 27.715676650995075,
        "lng": 85.39828266998909,
        "color": "#1d4ed8",
        "icon": "fa-bus",
        "iconType": "fontawesome",
        "id": 217
    },
    {
        "name": "Baba Chwok",
        "lat": 27.712475342393248,
        "lng": 85.39299861023258,
        "color": "#1d4ed8",
        "icon": "fa-bus",
        "iconType": "fontawesome",
        "id": 218
    },
    {
        "name": "Godhar Thapa Chwok",
        "lat": 27.707988024695876,
        "lng": 85.39035925506356,
        "color": "#1d4ed8",
        "icon": "fa-bus",
        "iconType": "fontawesome",
        "id": 219
    },
    {
        "name": "Khursani Chwok",
        "lat": 27.707602311302786,
        "lng": 85.38312796631084,
        "color": "#1d4ed8",
        "icon": "fa-bus",
        "iconType": "fontawesome",
        "id": 220
    },
    {
        "name": "Pipalbot Chwok",
        "lat": 27.706837622890966,
        "lng": 85.38015622121856,
        "color": "#1d4ed8",
        "icon": "fa-bus",
        "iconType": "fontawesome",
        "id": 221
    },
    {
        "name": "Adarsha Tol",
        "lat": 27.706928037075034,
        "lng": 85.37720612204342,
        "color": "#1d4ed8",
        "icon": "fa-bus",
        "iconType": "fontawesome",
        "id": 222
    },
    {
        "name": "Mantar Chwok",
        "lat": 27.70599281202735,
        "lng": 85.37071471648956,
        "color": "#1d4ed8",
        "icon": "fa-bus",
        "iconType": "fontawesome",
        "id": 223
    },
    {
        "name": "Tikathali Stop - Nepal Yatayat",
        "lat": 27.667319,
        "lng": 85.360486,
        "color": "#1d4ed8",
        "icon": "fa-bus",
        "iconType": "fontawesome",
        "id": 224
    },
    {
        "name": "Kharibot",
        "lat": 27.66501,
        "lng": 85.358936,
        "color": "#1d4ed8",
        "icon": "fa-bus",
        "iconType": "fontawesome",
        "id": 225
    },
    {
        "name": "Bhogateshwor Mahadev",
        "lat": 27.667865832080444,
        "lng": 85.35313986750288,
        "color": "#1d4ed8",
        "icon": "fa-bus",
        "iconType": "fontawesome",
        "id": 226
    },
    {
        "name": "Nagpokhari",
        "lat": 27.713323259683758,
        "lng": 85.32381780399187,
        "color": "#1d4ed8",
        "icon": "fa-bus",
        "iconType": "fontawesome",
        "id": 227
    },
    {
        "name": "Naxal",
        "lat": 27.713128,
        "lng": 85.326621,
        "color": "#1d4ed8",
        "icon": "fa-bus",
        "iconType": "fontawesome",
        "id": 228
    },
    {
        "name": "Chucchepati",
        "lat": 27.719136908102023,
        "lng": 85.3504843286376,
        "color": "#1d4ed8",
        "icon": "fa-bus",
        "iconType": "fontawesome",
        "id": 229
    },
    {
        "name": "Pani Tanki",
        "lat": 27.722151113835576,
        "lng": 85.3539009625667,
        "color": "#1d4ed8",
        "icon": "fa-bus",
        "iconType": "fontawesome",
        "id": 230
    },
    {
        "name": "Mahankal",
        "lat": 27.724316,
        "lng": 85.356925,
        "color": "#1d4ed8",
        "icon": "fa-bus",
        "iconType": "fontawesome",
        "id": 231
    },
    {
        "name": "Ramhiti Chwok",
        "lat": 27.725921,
        "lng": 85.362697,
        "color": "#1d4ed8",
        "icon": "fa-bus",
        "iconType": "fontawesome",
        "id": 232
    },
    {
        "name": "Kharibot Chwok",
        "lat": 27.735882076148975,
        "lng": 85.37706652464506,
        "color": "#1d4ed8",
        "icon": "fa-bus",
        "iconType": "fontawesome",
        "id": 233
    },
    {
        "name": "Haraharamahadev stop - Nepal Yatayat",
        "lat": 27.697304179539877,
        "lng": 85.38074539845395,
        "color": "#1d4ed8",
        "icon": "fa-bus",
        "iconType": "fontawesome",
        "id": 234
    },
    {
        "name": "Sunrise Chwok",
        "lat": 27.693076748711487,
        "lng": 85.36727044404377,
        "color": "#1d4ed8",
        "icon": "fa-bus",
        "iconType": "fontawesome",
        "id": 235
    },
    {
        "name": "Sai Ram Chwok",
        "lat": 27.689936923193407,
        "lng": 85.36332184859299,
        "color": "#1d4ed8",
        "icon": "fa-bus",
        "iconType": "fontawesome",
        "id": 236
    },
    {
        "name": "Ekantakuna",
        "lat": 27.66884418671191,
        "lng": 85.30630918707462,
        "color": "#1d4ed8",
        "icon": "fa-bus",
        "iconType": "fontawesome",
        "id": 237
    },
    {
        "name": "Kuleshwor",
        "lat": 27.692677653179327,
        "lng": 85.29841266287768,
        "color": "#1d4ed8",
        "icon": "fa-bus",
        "iconType": "fontawesome",
        "id": 238
    }
]
$json$::jsonb
);

select public.sync_routes(
$json$
[
    {
        "name": "Basundhara-RNAC",
        "stopIds": [
            14,
            1,
            2,
            3,
            4,
            5,
            6,
            9,
            10,
            8
        ],
        "color": "#1d4ed8",
        "style": "solid",
        "weight": 5,
        "snapToRoad": true,
        "ratingAverage": 0,
        "ratingCount": 0,
        "id": 1,
        "stops": [
            {
                "id": 1,
                "name": "Chakrapath"
            },
            {
                "id": 2,
                "name": "Teaching Hospital"
            },
            {
                "id": 3,
                "name": "Panipokhari"
            },
            {
                "id": 4,
                "name": "Lazimpat"
            },
            {
                "id": 5,
                "name": "Lainchaur"
            },
            {
                "id": 6,
                "name": "Jamal"
            },
            {
                "id": 7,
                "name": "Ghantaghar"
            },
            {
                "id": 8,
                "name": "Bhadrakali"
            },
            {
                "id": 9,
                "name": "Sahid Gate"
            },
            {
                "id": 10,
                "name": "RNAC"
            },
            {
                "id": 11,
                "name": "AI Yatayat"
            }
        ]
    },
    {
        "name": "Budhanilkantha-Ratnapark",
        "stopIds": [
            26,
            25,
            24,
            23,
            22,
            21,
            20,
            19,
            18,
            14,
            1,
            2,
            3,
            4,
            5,
            6,
            7
        ],
        "color": "#2f8e3f",
        "style": "solid",
        "weight": 2,
        "snapToRoad": true,
        "ratingAverage": 0,
        "ratingCount": 0,
        "id": 2
    },
    {
        "name": "Haraharamahadev  - Thapathali Loop - Thapathali",
        "stopIds": [
            234,
            235,
            236,
            153,
            98,
            115,
            207,
            177,
            135,
            45,
            59,
            35,
            36,
            131,
            132,
            201,
            120,
            103,
            92,
            211,
            100,
            237,
            166,
            41,
            238,
            106,
            194,
            209,
            201,
            132,
            131,
            36,
            35,
            59,
            45,
            135,
            177,
            207,
            115,
            98,
            153,
            236,
            235,
            234
        ],
        "color": "#4A2C2A",
        "style": "solid",
        "weight": 5,
        "snapToRoad": true,
        "ratingAverage": 0,
        "ratingCount": 0,
        "id": 3,
        "operator": [
            "Nepal Yatayat"
        ],
        "isVerifiedRoute": true
    },
    {
        "name": "Tikathali Stop - Kharibot ",
        "stopIds": [
            224,
            225,
            226,
            42,
            115,
            207,
            177,
            135,
            45,
            146,
            200,
            55,
            91,
            30,
            147,
            12,
            117,
            227,
            39,
            155,
            193,
            14,
            75,
            86,
            87,
            61,
            229,
            230,
            231,
            232,
            233
        ],
        "color": "#8B5523",
        "style": "solid",
        "weight": 5,
        "snapToRoad": true,
        "ratingAverage": 0,
        "ratingCount": 0,
        "id": 4,
        "operator": [
            "Nepal Yatayat"
        ],
        "isVerifiedRoute": true
    },
    {
        "name": "Nepal Yatayat Bus Station - Bhrikuti Mandap",
        "stopIds": [
            216,
            217,
            218,
            219,
            220,
            221,
            222,
            223,
            88,
            153,
            98,
            207,
            135,
            45,
            35,
            131,
            12,
            13,
            53
        ],
        "color": "#6F4E37",
        "style": "solid",
        "weight": 5,
        "snapToRoad": true,
        "ratingAverage": 0,
        "ratingCount": 0,
        "id": 5,
        "operator": [
            "Nepal Yatayat"
        ],
        "isVerifiedRoute": true
    },
    {
        "name": "Ringroad Loop - Mahanagar",
        "stopIds": [
            29,
            82,
            101,
            61,
            87,
            75,
            14,
            48,
            17,
            85,
            144,
            40,
            43,
            77,
            191,
            185,
            41,
            174,
            89,
            42,
            115,
            207,
            29
        ],
        "color": "#DC143C",
        "style": "solid",
        "weight": 5,
        "snapToRoad": true,
        "ratingAverage": 0,
        "ratingCount": 0,
        "id": 6,
        "operator": [
            "Mahanagar Yatayat"
        ],
        "isVerifiedRoute": true,
        "details": {
            "total_bus": 51,
            "distance_meter": 2180
        }
    },
    {
        "name": "Thankot - Dhulikhel",
        "stopIds": [
            74,
            44,
            167,
            99,
            28,
            190,
            162,
            188,
            157,
            203,
            64,
            213,
            125,
            115,
            29,
            82,
            101,
            61,
            87,
            75,
            14,
            48,
            17,
            85,
            144,
            40,
            43,
            191,
            185,
            105,
            76,
            138,
            173,
            199,
            208,
            198
        ],
        "color": "#FF4040",
        "style": "solid",
        "weight": 5,
        "snapToRoad": true,
        "ratingAverage": 0,
        "ratingCount": 0,
        "id": 7,
        "operator": [
            "Mahanagar Yatayat"
        ],
        "isVerifiedRoute": true,
        "details": {
            "distance_meter": 524100,
            "total_bus": 16
        }
    },
    {
        "name": "Lagankhel - Naya buspark",
        "stopIds": [
            122,
            119,
            100,
            70,
            92,
            120,
            201,
            209,
            8,
            109,
            124,
            2,
            193,
            14,
            48,
            17,
            85,
            144
        ],
        "color": "#228B22",
        "style": "solid",
        "weight": 5,
        "snapToRoad": true,
        "ratingAverage": 0,
        "ratingCount": 0,
        "id": 8,
        "operator": [
            "Sajha Yatayat"
        ],
        "isVerifiedRoute": true
    },
    {
        "name": "Lagankhel - Budhanilkantha",
        "stopIds": [
            122,
            119,
            100,
            70,
            92,
            120,
            209,
            8,
            5,
            4,
            2,
            193,
            14,
            18,
            20,
            22,
            24,
            71,
            26
        ],
        "color": "#50C878",
        "style": "solid",
        "weight": 5,
        "snapToRoad": true,
        "ratingAverage": 0,
        "ratingCount": 0,
        "id": 9,
        "operator": [
            "Sajha Yatayat"
        ],
        "isVerifiedRoute": true
    },
    {
        "name": "Tripureshwor - Godawari",
        "stopIds": [
            209,
            120,
            92,
            70,
            100,
            119,
            121,
            174,
            113,
            94,
            93,
            196,
            37,
            83
        ],
        "color": "#2E8B57",
        "style": "solid",
        "weight": 5,
        "snapToRoad": true,
        "ratingAverage": 0,
        "ratingCount": 0,
        "id": 10,
        "operator": [
            "Sajha Yatayat"
        ],
        "isVerifiedRoute": true
    },
    {
        "name": "Lagankhel - Lamatar",
        "stopIds": [
            209,
            120,
            92,
            70,
            100,
            119,
            121,
            174,
            89,
            158,
            116,
            108,
            165,
            126,
            123
        ],
        "color": "#4CBB17",
        "style": "solid",
        "weight": 5,
        "snapToRoad": true,
        "ratingAverage": 0,
        "ratingCount": 0,
        "id": 11,
        "operator": [
            "Sajha Yatayat"
        ],
        "isVerifiedRoute": true
    },
    {
        "name": "Thankot - Airport ",
        "stopIds": [
            198,
            208,
            199,
            173,
            138,
            76,
            105,
            161,
            186,
            106,
            194,
            209,
            8,
            184,
            55,
            146,
            52,
            181,
            29
        ],
        "color": "#31AA44",
        "style": "solid",
        "weight": 5,
        "snapToRoad": true,
        "ratingAverage": 0,
        "ratingCount": 0,
        "id": 12,
        "operator": [
            "Sajha Yatayat"
        ],
        "isVerifiedRoute": true
    },
    {
        "name": "Thankot - Budhanilkantha",
        "stopIds": [
            198,
            208,
            199,
            173,
            138,
            76,
            105,
            46,
            185,
            191,
            43,
            40,
            85,
            17,
            48,
            14,
            18,
            20,
            22,
            71,
            26
        ],
        "color": "#7FFF00",
        "style": "solid",
        "weight": 5,
        "snapToRoad": true,
        "ratingAverage": 0,
        "ratingCount": 0,
        "id": 13,
        "operator": [
            "Sajha Yatayat"
        ],
        "isVerifiedRoute": true
    },
    {
        "name": "Jamal - Lele",
        "stopIds": [
            5,
            8,
            209,
            120,
            92,
            70,
            100,
            119,
            174,
            114,
            73,
            202,
            156,
            204,
            214
        ],
        "color": "#263927",
        "style": "solid",
        "weight": 5,
        "snapToRoad": true,
        "ratingAverage": 0,
        "ratingCount": 0,
        "id": 14,
        "operator": [
            "Sajha Yatayat"
        ],
        "isVerifiedRoute": true
    },
    {
        "name": "Sankhu - RNAC",
        "stopIds": [
            170,
            171,
            96,
            69,
            197,
            136,
            139,
            102,
            58,
            61,
            101,
            82,
            160,
            68,
            78,
            12,
            38,
            9,
            8
        ],
        "color": "#9966CC",
        "style": "solid",
        "weight": 5,
        "snapToRoad": true,
        "ratingAverage": 0,
        "ratingCount": 0,
        "id": 15,
        "isVerifiedRoute": true,
        "details": {
            "duration_mins": 105
        }
    },
    {
        "name": "Purano Bus Park to Thimi",
        "stopIds": [
            9,
            50,
            184,
            131,
            35,
            45,
            177,
            207,
            115,
            97,
            125,
            213,
            212,
            64,
            203
        ],
        "color": "#233263",
        "style": "solid",
        "weight": 5,
        "snapToRoad": true,
        "ratingAverage": 0,
        "ratingCount": 0,
        "id": 16
    },
    {
        "name": "Purano Bus Park to Gairigaun Stop",
        "stopIds": [
            9,
            12,
            78,
            130,
            195,
            52,
            181,
            104,
            182,
            81
        ],
        "color": "#134d4e",
        "style": "solid",
        "weight": 5,
        "snapToRoad": true,
        "ratingAverage": 0,
        "ratingCount": 0,
        "id": 17
    },
    {
        "name": "Purano Bus Park to Panga Dobato",
        "stopIds": [
            9,
            209,
            106,
            41,
            118,
            192,
            142,
            205,
            143,
            57,
            150,
            137
        ],
        "color": "#58721e",
        "style": "solid",
        "weight": 5,
        "snapToRoad": true,
        "ratingAverage": 0,
        "ratingCount": 0,
        "id": 18
    },
    {
        "name": "Purano Bus Park to Gokarna Bus Stop",
        "stopIds": [
            9,
            54,
            12,
            117,
            68,
            90,
            160,
            82,
            101,
            61,
            95,
            58,
            102,
            32,
            84
        ],
        "color": "#796213",
        "style": "solid",
        "weight": 5,
        "snapToRoad": true,
        "ratingAverage": 0,
        "ratingCount": 0,
        "id": 19
    },
    {
        "name": "Purano Bus Park to Lagankhel Stop",
        "stopIds": [
            9,
            50,
            184,
            132,
            201,
            120,
            103,
            211,
            70,
            100,
            119,
            152,
            122
        ],
        "color": "#106eab",
        "style": "solid",
        "weight": 5,
        "snapToRoad": true,
        "ratingAverage": 0,
        "ratingCount": 0,
        "id": 20
    },
    {
        "name": "Ratna Park to Mangalbazar",
        "stopIds": [
            7,
            9,
            50,
            184,
            132,
            201,
            120,
            103,
            211,
            140,
            79,
            129
        ],
        "color": "#2d173c",
        "style": "solid",
        "weight": 5,
        "snapToRoad": true,
        "ratingAverage": 0,
        "ratingCount": 0,
        "id": 21
    },
    {
        "name": "Sundhara Stop to KattyaniChowk Stop",
        "stopIds": [
            189,
            56,
            7,
            9,
            54,
            147,
            30,
            183,
            91,
            55,
            200,
            146,
            178,
            168,
            134,
            111
        ],
        "color": "#41173a",
        "style": "solid",
        "weight": 5,
        "snapToRoad": true,
        "ratingAverage": 0,
        "ratingCount": 0,
        "id": 22
    },
    {
        "name": "Tempo Stop Near Old Baneshwar to Sundhara Stop",
        "stopIds": [
            195,
            31,
            159,
            176,
            178,
            146,
            200,
            55,
            91,
            183,
            30,
            147,
            54,
            9,
            10,
            189
        ],
        "color": "#732843",
        "style": "solid",
        "weight": 5,
        "snapToRoad": true,
        "ratingAverage": 0,
        "ratingCount": 0,
        "id": 23
    },
    {
        "name": "Purano Bus Park to Chapgaun",
        "stopIds": [
            9,
            50,
            184,
            132,
            201,
            120,
            103,
            211,
            70,
            100,
            119,
            152,
            122,
            49,
            121,
            174,
            63
        ],
        "color": "#194624",
        "style": "solid",
        "weight": 5,
        "snapToRoad": true,
        "ratingAverage": 0,
        "ratingCount": 0,
        "id": 24
    },
    {
        "name": "Sundhara Stop to Sankhamul Stop",
        "stopIds": [
            189,
            56,
            7,
            9,
            50,
            184,
            131,
            35,
            45,
            169
        ],
        "color": "#21514e",
        "style": "solid",
        "weight": 5,
        "snapToRoad": true,
        "ratingAverage": 0,
        "ratingCount": 0,
        "id": 25
    },
    {
        "name": "MAV School to Purano Bus Park",
        "stopIds": [
            113,
            174,
            121,
            49,
            122,
            152,
            119,
            100,
            70,
            211,
            103,
            120,
            201,
            132,
            184,
            50,
            10,
            189,
            56,
            5,
            9
        ],
        "color": "#2f1247",
        "style": "solid",
        "weight": 5,
        "snapToRoad": true,
        "ratingAverage": 0,
        "ratingCount": 0,
        "id": 26
    },
    {
        "name": "Shivapuri Park Stop to Purano Bus Park",
        "stopIds": [
            179,
            26,
            71,
            20,
            18,
            14,
            193,
            124,
            4,
            109,
            5,
            9
        ],
        "color": "#06374c",
        "style": "solid",
        "weight": 5,
        "snapToRoad": true,
        "ratingAverage": 0,
        "ratingCount": 0,
        "id": 27
    },
    {
        "name": "Balkumari Stop to Gopi Krishna Stop",
        "stopIds": [
            42,
            115,
            207,
            177,
            135,
            45,
            146,
            200,
            55,
            91,
            183,
            30,
            147,
            12,
            80,
            155,
            193,
            14,
            75,
            86
        ],
        "color": "#48124c",
        "style": "solid",
        "weight": 5,
        "snapToRoad": true,
        "ratingAverage": 0,
        "ratingCount": 0,
        "id": 28
    },
    {
        "name": "Patan Dhoka to Naxal Bhagwati",
        "stopIds": [
            151,
            103,
            120,
            201,
            132,
            184,
            50,
            10,
            189,
            56,
            109,
            4,
            80,
            155,
            39,
            141
        ],
        "color": "#491366",
        "style": "solid",
        "weight": 5,
        "snapToRoad": true,
        "ratingAverage": 0,
        "ratingCount": 0,
        "id": 29
    },
    {
        "name": "Teaching Hospital stop to ShivaDarshan Hall Stop",
        "stopIds": [
            193,
            14,
            75,
            86,
            87,
            61,
            101,
            82,
            195,
            31,
            159,
            176,
            178
        ],
        "color": "#2121a0",
        "style": "solid",
        "weight": 5,
        "snapToRoad": true,
        "ratingAverage": 0,
        "ratingCount": 0,
        "id": 30
    },
    {
        "name": "Naya Naikap to Koteshwor Stop",
        "stopIds": [
            145,
            106,
            209,
            201,
            132,
            131,
            35,
            45,
            177,
            207,
            115
        ],
        "color": "#105121",
        "style": "solid",
        "weight": 5,
        "snapToRoad": true,
        "ratingAverage": 0,
        "ratingCount": 0,
        "id": 31
    },
    {
        "name": "Bagbazar Stop to Kamal Binayak",
        "stopIds": [
            38,
            9,
            50,
            184,
            131,
            35,
            45,
            177,
            207,
            115,
            98,
            125,
            213,
            212,
            64,
            203,
            188,
            206,
            163,
            51,
            60,
            127,
            128,
            107
        ],
        "color": "#106c71",
        "style": "solid",
        "weight": 5,
        "snapToRoad": true,
        "ratingAverage": 0,
        "ratingCount": 0,
        "id": 32
    },
    {
        "name": "Panauti to Purano Bus Park",
        "stopIds": [
            149,
            44,
            167,
            99,
            28,
            190,
            47,
            66,
            162,
            206,
            188,
            203,
            64,
            212,
            213,
            125,
            98,
            115,
            207,
            177,
            45,
            35,
            131,
            184,
            50,
            9
        ],
        "color": "#7c502d",
        "style": "solid",
        "weight": 5,
        "snapToRoad": true,
        "ratingAverage": 0,
        "ratingCount": 0,
        "id": 33
    },
    {
        "name": "Bagbazar Stop to Changu Narayan",
        "stopIds": [
            38,
            9,
            50,
            184,
            131,
            35,
            45,
            177,
            207,
            115,
            98,
            125,
            213,
            212,
            64,
            203,
            188,
            206,
            163,
            51,
            60,
            127,
            62
        ],
        "color": "#156d1a",
        "style": "solid",
        "weight": 5,
        "snapToRoad": true,
        "ratingAverage": 0,
        "ratingCount": 0,
        "id": 34
    },
    {
        "name": "Dhulikhel Stop to Purano Bus Park",
        "stopIds": [
            74,
            44,
            167,
            99,
            28,
            190,
            47,
            66,
            162,
            206,
            188,
            203,
            64,
            212,
            213,
            125,
            98,
            115,
            207,
            177,
            45,
            35,
            131,
            184,
            50,
            9
        ],
        "color": "#477420",
        "style": "solid",
        "weight": 5,
        "snapToRoad": true,
        "ratingAverage": 0,
        "ratingCount": 0,
        "id": 35
    },
    {
        "name": "Purano Bus Park to Chobhar Gate",
        "stopIds": [
            9,
            209,
            106,
            41,
            118,
            192,
            142,
            65
        ],
        "color": "#131146",
        "style": "solid",
        "weight": 5,
        "snapToRoad": true,
        "ratingAverage": 0,
        "ratingCount": 0,
        "id": 36
    },
    {
        "name": "Purano Bus Park to Chyamasingh",
        "stopIds": [
            9,
            209,
            201,
            132,
            131,
            35,
            45,
            177,
            207,
            115,
            98,
            125,
            213,
            212,
            64,
            203,
            188,
            206,
            162,
            66,
            47,
            190,
            28,
            99,
            67
        ],
        "color": "#834a25",
        "style": "solid",
        "weight": 5,
        "snapToRoad": true,
        "ratingAverage": 0,
        "ratingCount": 0,
        "id": 37
    },
    {
        "name": "Bhaktapur Bus Park to Bagbazar Stop",
        "stopIds": [
            51,
            148,
            34,
            172,
            153,
            115,
            207,
            177,
            135,
            45,
            59,
            35,
            36,
            131,
            184,
            12,
            38
        ],
        "color": "#56132d",
        "style": "solid",
        "weight": 5,
        "snapToRoad": true,
        "ratingAverage": 0,
        "ratingCount": 0,
        "id": 38
    },
    {
        "name": "Dhokashi to Balkhu",
        "stopIds": [
            72,
            65,
            142,
            192,
            118,
            41
        ],
        "color": "#0d5e34",
        "style": "solid",
        "weight": 5,
        "snapToRoad": true,
        "ratingAverage": 0,
        "ratingCount": 0,
        "id": 39
    },
    {
        "name": "Balaju to Jamal",
        "stopIds": [
            40,
            187,
            180,
            27,
            4,
            109,
            5
        ],
        "color": "#591742",
        "style": "solid",
        "weight": 5,
        "snapToRoad": true,
        "ratingAverage": 0,
        "ratingCount": 0,
        "id": 40
    },
    {
        "name": "Balkhu to Sankhamul Stop",
        "stopIds": [
            41,
            106,
            209,
            201,
            132,
            131,
            35,
            45,
            169
        ],
        "color": "#788e2a",
        "style": "solid",
        "weight": 5,
        "snapToRoad": true,
        "ratingAverage": 0,
        "ratingCount": 0,
        "id": 41
    },
    {
        "name": "Gokarna Bus Stop to Kalanki",
        "stopIds": [
            84,
            32,
            102,
            58,
            95,
            61,
            87,
            86,
            75,
            14,
            48,
            17,
            144,
            40,
            77,
            105
        ],
        "color": "#0f7615",
        "style": "solid",
        "weight": 5,
        "snapToRoad": true,
        "ratingAverage": 0,
        "ratingCount": 0,
        "id": 42
    },
    {
        "name": "Babarmahal Stop to Maitighar",
        "stopIds": [
            35,
            45,
            177,
            207,
            115,
            42,
            89,
            33,
            174,
            121,
            49,
            122,
            152,
            119,
            100,
            211,
            103,
            120,
            201,
            132,
            131
        ],
        "color": "#254878",
        "style": "solid",
        "weight": 5,
        "snapToRoad": true,
        "ratingAverage": 0,
        "ratingCount": 0,
        "id": 43
    },
    {
        "name": "Matatirtha Stop to Kapan Stop",
        "stopIds": [
            133,
            173,
            138,
            105,
            186,
            106,
            209,
            201,
            132,
            184,
            12,
            117,
            68,
            90,
            160,
            82,
            101,
            61,
            87,
            215,
            110
        ],
        "color": "#3a5122",
        "style": "solid",
        "weight": 5,
        "snapToRoad": true,
        "ratingAverage": 0,
        "ratingCount": 0,
        "id": 44
    },
    {
        "name": "Kapan Stop to Ratna Park",
        "stopIds": [
            110,
            215,
            87,
            61,
            101,
            82,
            195,
            130,
            78,
            12,
            7
        ],
        "color": "#83128d",
        "style": "solid",
        "weight": 5,
        "snapToRoad": true,
        "ratingAverage": 0,
        "ratingCount": 0,
        "id": 45
    }
]
$json$::jsonb
);

-- passengers intentionally omitted from payload; sync function will use default handling
select public.sync_vehicles(
$json$
[
    {
        "name":  "Budhanilkantha Yatayat 5588",
        "lat":  27.748265485282406,
        "lng":  85.34441471099854,
        "routeId":  2,
        "speed":  28,
        "color":  "#1d87d7",
        "icon":  "mayur-yatayat.jpg",
        "iconType":  "image",
        "moving":  false,
        "bearing":  0,
        "ratingAverage":  4,
        "ratingCount":  1,
        "id":  1,
        "vehicle_image":  "assets/mayur-yatayat.jpg"
    },
    {
        "name":  "Nepal Yatayat #1",
        "lat":  27.698243870022573,
        "lng":  85.38012289378132,
        "routeId":  3,
        "speed":  31,
        "color":  "#4a2c2a",
        "icon":  "nepal-yatayat.jpg",
        "iconType":  "image",
        "vehicle_image":  "assets/nepal-yatayat.jpg",
        "moving":  false,
        "bearing":  213,
        "ratingAverage":  0,
        "ratingCount":  0,
        "id":  2
    },
    {
        "name":  "Nepal Yatayat #2",
        "lat":  27.69059150531453,
        "lng":  85.33316439886549,
        "routeId":  4,
        "speed":  27,
        "color":  "#8b5523",
        "icon":  "nepal-yatayat.jpg",
        "iconType":  "image",
        "vehicle_image":  "assets/nepal-yatayat.jpg",
        "moving":  false,
        "bearing":  354,
        "ratingAverage":  0,
        "ratingCount":  0,
        "id":  3
    },
    {
        "name":  "Nepal Yatayat #3",
        "lat":  27.698120147680104,
        "lng":  85.32111167907716,
        "routeId":  5,
        "speed":  36,
        "color":  "#6f4e37",
        "icon":  "nepal-yatayat.jpg",
        "iconType":  "image",
        "vehicle_image":  "assets/nepal-yatayat.jpg",
        "moving":  false,
        "bearing":  35,
        "ratingAverage":  0,
        "ratingCount":  0,
        "id":  4
    },
    {
        "name":  "Mahanagar Yatayat #4",
        "lat":  27.700411297161544,
        "lng":  85.3533515102186,
        "routeId":  6,
        "speed":  37,
        "color":  "#dc143c",
        "icon":  "mahanagar-yatayat.jpg",
        "iconType":  "image",
        "vehicle_image":  "assets/mahanagar-yatayat.jpg",
        "moving":  false,
        "bearing":  239,
        "ratingAverage":  0,
        "ratingCount":  0,
        "id":  5
    },
    {
        "name":  "Mahanagar Yatayat #5",
        "lat":  27.720486677392646,
        "lng":  85.3453709889472,
        "routeId":  7,
        "speed":  27,
        "color":  "#FF4040",
        "icon":  "nepal-offroad-bus.jpg",
        "iconType":  "image",
        "vehicle_image":  "assets/nepal-offroad-bus.jpg",
        "moving":  false,
        "bearing":  200,
        "ratingAverage":  0,
        "ratingCount":  0,
        "id":  6
    },
    {
        "name":  "Sajha Yatayat #6",
        "lat":  27.66616411353022,
        "lng":  85.3217013770367,
        "routeId":  8,
        "speed":  27,
        "color":  "#228b22",
        "icon":  "sajha-yatayat.jpg",
        "iconType":  "image",
        "vehicle_image":  "assets/sajha-yatayat.jpg",
        "moving":  false,
        "bearing":  35,
        "ratingAverage":  0,
        "ratingCount":  0,
        "id":  7
    },
    {
        "name":  "Sajha Yatayat #7",
        "lat":  27.673428960531623,
        "lng":  85.31330702304308,
        "routeId":  9,
        "speed":  28,
        "color":  "#50c878",
        "icon":  "sajha-yatayat.jpg",
        "iconType":  "image",
        "vehicle_image":  "assets/sajha-yatayat.jpg",
        "moving":  false,
        "bearing":  146,
        "ratingAverage":  0,
        "ratingCount":  0,
        "id":  8
    },
    {
        "name":  "Sajha Yatayat #8",
        "lat":  27.67300956049057,
        "lng":  85.31394385453773,
        "routeId":  10,
        "speed":  23,
        "color":  "#2e8b57",
        "icon":  "sajha-yatayat.jpg",
        "iconType":  "image",
        "vehicle_image":  "assets/sajha-yatayat.jpg",
        "moving":  false,
        "bearing":  165,
        "ratingAverage":  0,
        "ratingCount":  0,
        "id":  9
    },
    {
        "name":  "Sajha Yatayat #9",
        "lat":  27.66233042768478,
        "lng":  85.32331451017019,
        "routeId":  11,
        "speed":  21,
        "color":  "#4cbb17",
        "icon":  "sajha-yatayat.jpg",
        "iconType":  "image",
        "vehicle_image":  "assets/sajha-yatayat.jpg",
        "moving":  false,
        "bearing":  359,
        "ratingAverage":  0,
        "ratingCount":  0,
        "id":  10
    },
    {
        "name":  "Sajha Yatayat #10",
        "lat":  27.69683419397905,
        "lng":  85.30526765863054,
        "routeId":  12,
        "speed":  33,
        "color":  "#31aa44",
        "icon":  "sajha-yatayat.jpg",
        "iconType":  "image",
        "vehicle_image":  "assets/sajha-yatayat.jpg",
        "moving":  false,
        "bearing":  259,
        "ratingAverage":  0,
        "ratingCount":  0,
        "id":  11
    },
    {
        "name":  "Sajha Yatayat #11",
        "lat":  27.740889210274034,
        "lng":  85.3362898594708,
        "routeId":  13,
        "speed":  26,
        "color":  "#7fff00",
        "icon":  "sajha-yatayat.jpg",
        "iconType":  "image",
        "vehicle_image":  "assets/sajha-yatayat.jpg",
        "moving":  false,
        "bearing":  120,
        "ratingAverage":  0,
        "ratingCount":  0,
        "id":  12
    },
    {
        "name":  "Sajha Yatayat #12",
        "lat":  27.597989966124228,
        "lng":  85.32430674792907,
        "routeId":  14,
        "speed":  29,
        "color":  "#263927",
        "icon":  "sajha-yatayat.jpg",
        "iconType":  "image",
        "vehicle_image":  "assets/sajha-yatayat.jpg",
        "moving":  false,
        "bearing":  264,
        "ratingAverage":  0,
        "ratingCount":  0,
        "id":  13
    }
]
$json$::jsonb
);

select public.sync_suggestions(
$json$
[
    {
        "id": 1,
        "name": "Zenith Kandel",
        "category": "missing_stop",
        "message": "basundhara-rnac stop misses the kalanki stop, please fix",
        "task": {
            "action": "add_stop_to_route",
            "summary": "Add Kalanki stop to Basundhara-RNAC route",
            "entity_type": "stop",
            "entity_id": 105,
            "entity_name": "Kalanki",
            "details": {
                "route_id": 1,
                "route_name": "Basundhara-RNAC",
                "stop_id": 105,
                "stop_name": "Kalanki"
            }
        },
        "status": "approved",
        "created_at": "2026-03-14T11:49:11+01:00",
        "resolved_at": "2026-03-14T11:49:30+01:00"
    },
    {
        "id": 2,
        "name": "kailash magar",
        "category": "missing_stop",
        "message": "kalanki is missing in (basundhara to rnac) route",
        "task": {
            "action": "add_stop_to_route",
            "summary": "Add Kalanki stop to Basundhara-RNAC route",
            "entity_type": "stop",
            "entity_id": 105,
            "entity_name": "Kalanki",
            "details": {
                "route_id": 1,
                "route_name": "Basundhara-RNAC",
                "stop_id": 105,
                "stop_name": "Kalanki"
            }
        },
        "status": "completed",
        "created_at": "2026-03-14T11:55:11+01:00",
        "resolved_at": "2026-03-14T11:55:31+01:00"
    }
]
$json$::jsonb
);

select public.sync_obstructions(
$json$
[]
$json$::jsonb
);

commit;
