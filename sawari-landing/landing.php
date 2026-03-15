<?php
$envFilePrimary = __DIR__ . '/../app/.env';
$envFileLocal = __DIR__ . '/.env';
$env = [];

$envFile = file_exists($envFilePrimary) ? $envFilePrimary : $envFileLocal;
if (file_exists($envFile)) {
    foreach (file($envFile, FILE_IGNORE_NEW_LINES | FILE_SKIP_EMPTY_LINES) as $line) {
        $trimmed = trim($line);
        if ($trimmed === '' || str_starts_with($trimmed, '#')) {
            continue;
        }
        if (strpos($trimmed, '=') === false) {
            continue;
        }
        [$key, $value] = explode('=', $trimmed, 2);
        $env[trim($key)] = trim($value);
    }
}

$groqApiKey = $env['GROQ_API_KEY'] ?? '';
?>
<!doctype html>
<html lang="en">

<head>
    <meta charset="UTF-8" />
    <meta name="viewport" content="width=device-width, initial-scale=1.0" />
    <title>Sawari Landing</title>
    <meta name="description"
        content="Minimal and practical landing page for Sawari, Kathmandu Valley public transit navigator." />
    <link rel="preconnect" href="https://fonts.googleapis.com" />
    <link rel="preconnect" href="https://fonts.gstatic.com" crossorigin />
    <link
        href="https://fonts.googleapis.com/css2?family=Fraunces:opsz,wght@9..144,400;9..144,600;9..144,700&family=Work+Sans:wght@400;500;600&display=swap"
        rel="stylesheet" />
    <link rel="stylesheet" href="https://cdnjs.cloudflare.com/ajax/libs/font-awesome/6.5.1/css/all.min.css" />
    <link rel="stylesheet" href="landing.css" />
</head>

<body>
    <header class="topbar" id="top">
        <a href="#top" class="brand">Sawari</a>
        <nav class="topnav" aria-label="Main">
            <a href="#about">About</a>
            <a href="#how">How it works</a>
            <a href="#features">Features</a>
            <a href="#gallery">Gallery</a>
            <a href="#ask">Ask</a>
            <a href="#contribute">Contribute</a>
            <a href="../app/index.php" class="btn-nav">Open Navigator</a>
        </nav>
    </header>

    <main>
        <section class="hero section-reveal">
            <div class="hero-copy">
                <p class="eyebrow">Kathmandu Valley Transit Companion</p>
                <h1>Find the right bus in less time.</h1>
                <p>
                    Sawari helps you move through Kathmandu with route planning, transfer support,
                    fare estimates, and nearby stop discovery. It is built for local travel habits,
                    not generic map assumptions.
                </p>
                <div class="hero-cta">
                    <a class="btn-main" href="../app/index.php">Plan a Journey</a>
                    <a class="btn-ghost" href="#how">See How it Works</a>
                </div>
            </div>
            <div class="hero-mosaic" aria-label="Transit photos around Kathmandu">
                <img src="gallery/landing.jpg" alt="Kathmandu public transport scene" loading="eager" />
                <img src="gallery/hiace.webp" alt="Hiace microbus" loading="lazy" />
                <img src="gallery/mahanagar.jpg" alt="Mahanagar yatayat bus" loading="lazy" />
                <img src="gallery/sajha-bus-EV.jpg" alt="Electric Sajha bus" loading="lazy" />
            </div>
        </section>

        <section id="about" class="section section-reveal">
            <div class="section-head">
                <h2>What Sawari does</h2>
                <p>All practical tools from the existing landing, redesigned with a cleaner visual system.</p>
            </div>
            <div class="about-grid">
                <article>
                    <h3>Built for Kathmandu</h3>
                    <p>
                        Route data reflects real operators, common transfer behavior, and stop names used by
                        everyday commuters. You can start from plain place names and still get practical options.
                    </p>
                </article>
                <article>
                    <h3>No app needed</h3>
                    <p>
                        The navigator runs in your browser. No account, no mandatory signup, no mobile install.
                        Open, search, and travel.
                    </p>
                </article>
                <article>
                    <h3>Community-corrected</h3>
                    <p>
                        Riders can submit route corrections and missing stops. Admin review turns local knowledge
                        into better routing for everyone.
                    </p>
                </article>
            </div>
            <div class="photo-ribbon photo-ribbon-6">
                <img src="gallery/image1.jpg" alt="Transit photo 1" loading="lazy" />
                <img src="gallery/image2.jpg" alt="Transit photo 2" loading="lazy" />
                <img src="gallery/image3.jpg" alt="Transit photo 3" loading="lazy" />
                <img src="gallery/image4.jpg" alt="Transit photo 4" loading="lazy" />
                <img src="gallery/image5.jpg" alt="Transit photo 5" loading="lazy" />
                <img src="gallery/image6.jpg" alt="Transit photo 6" loading="lazy" />
            </div>
        </section>

        <section id="how" class="section section-tint section-reveal">
            <div class="section-head">
                <h2>How it works</h2>
            </div>
            <div class="steps">
                <article>
                    <span>01</span>
                    <h3>Tell Sawari where you are going</h3>
                    <p>Type origin and destination in plain language, or pick points on the map.</p>
                </article>
                <article>
                    <span>02</span>
                    <h3>Get direct or transfer routes</h3>
                    <p>The planner checks nearby stops and shared stops to suggest valid journey paths.</p>
                </article>
                <article>
                    <span>03</span>
                    <h3>Review fare and walking legs</h3>
                    <p>Estimated fares and short walking segments are shown with route details.</p>
                </article>
                <article>
                    <span>04</span>
                    <h3>Track nearby buses where available</h3>
                    <p>Live vehicle updates help you decide when to board and which stop to use.</p>
                </article>
            </div>
        </section>

        <section id="features" class="section section-reveal">
            <div class="section-head">
                <h2>Everything included</h2>
            </div>
            <div class="feature-grid">
                <article class="feature-card">
                    <h3><i class="fa-solid fa-route"></i> Route Planning</h3>
                    <p>Direct and transfer routes with stop-level matching.</p>
                </article>
                <article class="feature-card">
                    <h3><i class="fa-solid fa-ticket"></i> Fare Estimation</h3>
                    <p>Fare calculation aligned with local pricing rules and rounding.</p>
                </article>
                <article class="feature-card">
                    <h3><i class="fa-solid fa-wand-magic-sparkles"></i> Natural Language</h3>
                    <p>Understands common phrases and location input from users.</p>
                </article>
                <article class="feature-card">
                    <h3><i class="fa-solid fa-location-arrow"></i> GPS and Nearby Stops</h3>
                    <p>Uses your location to find reachable stops around you.</p>
                </article>
                <article class="feature-card">
                    <h3><i class="fa-solid fa-leaf"></i> Carbon Comparison</h3>
                    <p>Shows approximate CO2 savings for public transport use.</p>
                </article>
                <article class="feature-card">
                    <h3><i class="fa-solid fa-circle-half-stroke"></i> Dark and Light Themes</h3>
                    <p>Theme modes for clearer map reading day or night.</p>
                </article>
            </div>
        </section>

        <section id="gallery" class="section section-reveal">
            <div class="section-head">
                <h2>Gallery of daily transit life</h2>
                <p>A wide image field from the provided collection.</p>
            </div>
            <div class="gallery-grid">
                <img src="gallery/1594641445.jpg" alt="Kathmandu road transit" loading="lazy" />
                <img src="gallery/302216934_482557370551393_6475660738973236802_n.jpg" alt="Bus service image"
                    loading="lazy" />
                <img src="gallery/39089002_214055306121394_3987346834723962880_n_mmZU29H7GE.jpg" alt="Older bus photo"
                    loading="lazy" />
                <img src="gallery/473749570_1355280112583010_593412764911101796_n.jpg" alt="Public bus photograph"
                    loading="lazy" />
                <img src="gallery/473783154_1062778689195922_6904992953434292580_n.jpg" alt="Transport route image"
                    loading="lazy" />
                <img src="gallery/481820024_656265493582501_1243983797300762177_n.jpg" alt="Modern local bus"
                    loading="lazy" />
                <img src="gallery/81038617_2710162452340155_22013957954863104_n.jpg" alt="Transit community photo"
                    loading="lazy" />
                <img src="gallery/digo-yatayat.jpg" alt="Digo yatayat bus" loading="lazy" />
                <img src="gallery/high-tech-buses.jpg" alt="High tech buses" loading="lazy" />
                <img src="gallery/image7.jpg" alt="Transit photo 7" loading="lazy" />
                <img src="gallery/IMG-1.jpeg" alt="Transit photo Kathmandu" loading="lazy" />
                <img src="gallery/mahanagar-yatayat.png" alt="Mahanagar yatayat fleet" loading="lazy" />
                <img src="gallery/manangar-yatayat.jpg" alt="Yatayat bus line" loading="lazy" />
                <img src="gallery/mayur.jpg" alt="Mayur bus" loading="lazy" />
                <img src="gallery/mayur_yatayat_XWhLImWBJm.jpg" alt="Mayur yatayat route" loading="lazy" />
                <img src="gallery/nepal.jpg" alt="Nepal transit road" loading="lazy" />
                <img src="gallery/nilo-micro.jpg" alt="Blue microbus" loading="lazy" />
                <img src="gallery/routes.jpg" alt="Route board" loading="lazy" />
                <img src="gallery/sajha.jpg" alt="Sajha bus image" loading="lazy" />
                <img src="gallery/sajha1.jpg" alt="Sajha route" loading="lazy" />
                <img src="gallery/SajhaBus_20220707134627.jpg" alt="Sajha bus older photo" loading="lazy" />
                <img src="gallery/sajha-yatayat-kathmandu-narayangadh-shuttle-service-1758693554.jpg"
                    alt="Sajha shuttle bus" loading="lazy" />
                <img src="gallery/samakhusi-yatayat-1729569860.jpg" alt="Samakhusi route bus" loading="lazy" />
                <img src="gallery/tempo.jpg" alt="Tempo transport" loading="lazy" />
            </div>
        </section>

        <section id="ask" class="section section-tint section-reveal">
            <div class="section-head">
                <h2>Ask Sawari</h2>
                <p>Transit questions answered in simple language.</p>
            </div>
            <div class="chat-shell">
                <div class="chat-log" id="chat-log">
                    <div class="chat-msg assistant">
                        <div class="chat-avatar"><i class="fa-solid fa-bus"></i></div>
                        <div class="chat-bubble">
                            Namaste. Ask about routes, fares, or which stop to use.
                        </div>
                    </div>
                </div>
                <form id="chat-form" class="chat-form">
                    <input type="text" id="chat-input" placeholder="Example: Kalanki to Chabahil by bus"
                        autocomplete="off" />
                    <button type="submit" id="chat-send" aria-label="Send"><i
                            class="fa-solid fa-paper-plane"></i></button>
                </form>
            </div>
        </section>

        <section id="contribute" class="section section-reveal">
            <div class="section-head">
                <h2>Help improve Sawari</h2>
                <p>Send missing stops, route corrections, and fare updates.</p>
            </div>
            <div class="contrib-grid">
                <form id="suggest-form" class="suggest-form">
                    <label for="suggest-name">Your Name <span>(optional)</span></label>
                    <input id="suggest-name" type="text" maxlength="50" placeholder="Anonymous" />

                    <label for="suggest-category">Category</label>
                    <select id="suggest-category">
                        <option value="route_correction">Route Correction</option>
                        <option value="missing_stop">Missing Stop</option>
                        <option value="fare_issue">Fare Issue</option>
                        <option value="new_route">New Route Suggestion</option>
                        <option value="general" selected>General Feedback</option>
                    </select>

                    <label for="suggest-message">Suggestion</label>
                    <textarea id="suggest-message" rows="5" maxlength="1000" placeholder="Share details here"
                        required></textarea>
                    <div class="count"><span id="suggest-chars">0</span> / 1000</div>

                    <button id="suggest-submit" type="submit"><i class="fa-solid fa-paper-plane"></i> Submit
                        Suggestion</button>
                    <div id="suggest-feedback" class="suggest-feedback" style="display:none"></div>
                </form>

                <aside class="contrib-note">
                    <h3>Review flow</h3>
                    <ol>
                        <li>Suggestion is parsed and logged for admin review.</li>
                        <li>Admin validates it with ground reality and route data.</li>
                        <li>Approved fixes are added to the transit dataset.</li>
                    </ol>
                    <p>Your identity is optional. No email is required.</p>
                    <p class="quick-links">
                        <a href="../app/index.php">Navigator</a>
                        <a href="../app/admin/">Admin</a>
                    </p>
                </aside>
            </div>
        </section>
    </main>

    <footer class="site-footer">
        <p>Sawari by Team Spark</p>
        <p>&copy; <?= date('Y') ?> All rights reserved</p>
    </footer>

    <script>const GROQ_API_KEY = <?= json_encode($groqApiKey) ?>;</script>
    <script src="landing.js"></script>
</body>

</html>