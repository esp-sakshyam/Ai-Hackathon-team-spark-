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
    <meta name="description" content="Animated, image-rich landing page for Sawari public transit in Kathmandu Valley." />
    <link rel="preconnect" href="https://fonts.googleapis.com" />
    <link rel="preconnect" href="https://fonts.gstatic.com" crossorigin />
    <link href="https://fonts.googleapis.com/css2?family=Fraunces:opsz,wght@9..144,400;9..144,600;9..144,700&family=Plus+Jakarta+Sans:wght@400;500;600;700&display=swap" rel="stylesheet" />
    <link rel="stylesheet" href="https://cdnjs.cloudflare.com/ajax/libs/font-awesome/6.5.1/css/all.min.css" />
    <link rel="stylesheet" href="landing.css" />
</head>

<body>
    <div class="texture-overlay" aria-hidden="true"></div>
    <div class="mesh-gradient" aria-hidden="true"></div>
    <div class="particle-layer" id="particle-layer" aria-hidden="true"></div>

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
        <section class="hero section-reveal" id="hero">
            <div class="hero-bg" style="background-image: url('gallery/landing.jpg');" aria-hidden="true"></div>
            <div class="hero-blob hero-blob-1" aria-hidden="true"></div>
            <div class="hero-blob hero-blob-2" aria-hidden="true"></div>
            <div class="hero-blob hero-blob-3" aria-hidden="true"></div>

            <div class="hero-card">
                <p class="eyebrow">Kathmandu Valley Transit Companion</p>
                <h1>Local transit, beautifully mapped.</h1>
                <p>
                    Plan your bus journey with practical route guidance, transfer intelligence, fare estimates,
                    and nearby stop discovery built around how Kathmandu actually moves.
                </p>
                <div class="hero-cta">
                    <a class="btn-main" href="../app/index.php">Plan a Journey</a>
                    <a class="btn-ghost" href="#how">Explore the System</a>
                </div>
                <div class="hero-chip-row">
                    <span><i class="fa-solid fa-route"></i> Route planner</span>
                    <span><i class="fa-solid fa-bus"></i> Local operators</span>
                    <span><i class="fa-solid fa-map-location-dot"></i> Nearby stops</span>
                </div>
            </div>

            <div class="hero-side-grid" aria-label="Selected transit visuals">
                <img src="gallery/sajha-bus-EV.jpg" alt="Electric Sajha bus" loading="lazy" />
                <img src="gallery/hiace.webp" alt="Hiace microbus" loading="lazy" />
                <img src="gallery/mahanagar.jpg" alt="Mahanagar bus" loading="lazy" />
                <img src="gallery/digo-yatayat.jpg" alt="Digo yatayat" loading="lazy" />
            </div>
        </section>

        <div class="svg-separator" aria-hidden="true">
            <svg viewBox="0 0 1440 90" preserveAspectRatio="none">
                <path d="M0,43 C180,76 360,8 540,30 C720,52 900,84 1080,58 C1260,32 1360,8 1440,21 L1440,90 L0,90 Z"></path>
            </svg>
        </div>

        <section id="about" class="section section-reveal">
            <div class="section-head">
                <h2>What Sawari does</h2>
                <p>All core capabilities stay intact, now presented with richer motion and cleaner hierarchy.</p>
            </div>
            <div class="about-grid">
                <article>
                    <h3>Built for Kathmandu routes</h3>
                    <p>
                        Sawari models local route behavior and common stop names across Kathmandu, Lalitpur,
                        and Bhaktapur, so commuters can search naturally.
                    </p>
                </article>
                <article>
                    <h3>No app required</h3>
                    <p>
                        Everything works from browser only. Open the navigator, enter points, and start moving.
                    </p>
                </article>
                <article>
                    <h3>Community-corrected data</h3>
                    <p>
                        Riders can submit missing stops and route corrections that flow to admin verification.
                    </p>
                </article>
            </div>

            <div class="photo-ribbon photo-ribbon-7">
                <img src="gallery/image1.jpg" alt="Transit photo 1" loading="lazy" />
                <img src="gallery/image2.jpg" alt="Transit photo 2" loading="lazy" />
                <img src="gallery/image3.jpg" alt="Transit photo 3" loading="lazy" />
                <img src="gallery/image4.jpg" alt="Transit photo 4" loading="lazy" />
                <img src="gallery/image5.jpg" alt="Transit photo 5" loading="lazy" />
                <img src="gallery/image6.jpg" alt="Transit photo 6" loading="lazy" />
                <img src="gallery/image7.jpg" alt="Transit photo 7" loading="lazy" />
            </div>
        </section>

        <section id="how" class="section section-tint section-reveal">
            <div class="section-head">
                <h2>How it works</h2>
            </div>
            <div class="steps">
                <article>
                    <span>01</span>
                    <h3>Enter origin and destination</h3>
                    <p>Use natural language or map points to start your journey query.</p>
                </article>
                <article>
                    <span>02</span>
                    <h3>See direct and transfer routes</h3>
                    <p>Sawari checks nearby stop networks and suggests practical combinations.</p>
                </article>
                <article>
                    <span>03</span>
                    <h3>Check fare and walking legs</h3>
                    <p>Fare estimates and short walk segments are shown with clear trip structure.</p>
                </article>
                <article>
                    <span>04</span>
                    <h3>Track nearby buses when available</h3>
                    <p>Live bus updates support better timing and boarding choices.</p>
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
                    <p>Direct and transfer route discovery using stop-level matching.</p>
                </article>
                <article class="feature-card">
                    <h3><i class="fa-solid fa-ticket"></i> Fare Estimation</h3>
                    <p>Fare approximation aligned with local tariff logic.</p>
                </article>
                <article class="feature-card">
                    <h3><i class="fa-solid fa-wand-magic-sparkles"></i> Natural Language</h3>
                    <p>Supports practical everyday phrasing from users.</p>
                </article>
                <article class="feature-card">
                    <h3><i class="fa-solid fa-location-arrow"></i> GPS and Nearby Stops</h3>
                    <p>Locates surrounding stops and available passing routes.</p>
                </article>
                <article class="feature-card">
                    <h3><i class="fa-solid fa-leaf"></i> Carbon Comparison</h3>
                    <p>Displays approximate CO2 savings for public transit use.</p>
                </article>
                <article class="feature-card">
                    <h3><i class="fa-solid fa-circle-half-stroke"></i> Dark and Light Themes</h3>
                    <p>Theme options for readability in different lighting conditions.</p>
                </article>
            </div>
        </section>

        <section id="gallery" class="section section-reveal">
            <div class="section-head">
                <h2>Gallery of transit life</h2>
                <p>A large visual field using your existing collection.</p>
            </div>
            <div class="gallery-grid">
                <img src="gallery/1594641445.jpg" alt="Kathmandu road transit" loading="lazy" />
                <img src="gallery/302216934_482557370551393_6475660738973236802_n.jpg" alt="Bus service image" loading="lazy" />
                <img src="gallery/39089002_214055306121394_3987346834723962880_n_mmZU29H7GE.jpg" alt="Older bus photo" loading="lazy" />
                <img src="gallery/473749570_1355280112583010_593412764911101796_n.jpg" alt="Public bus photograph" loading="lazy" />
                <img src="gallery/473783154_1062778689195922_6904992953434292580_n.jpg" alt="Transport route image" loading="lazy" />
                <img src="gallery/481820024_656265493582501_1243983797300762177_n.jpg" alt="Modern local bus" loading="lazy" />
                <img src="gallery/81038617_2710162452340155_22013957954863104_n.jpg" alt="Transit community photo" loading="lazy" />
                <img src="gallery/high-tech-buses.jpg" alt="High tech buses" loading="lazy" />
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
                <img src="gallery/sajha-yatayat-kathmandu-narayangadh-shuttle-service-1758693554.jpg" alt="Sajha shuttle bus" loading="lazy" />
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
                            Namaste. Ask me about routes, fares, or nearby stops in Kathmandu Valley.
                        </div>
                    </div>
                </div>
                <form id="chat-form" class="chat-form">
                    <input type="text" id="chat-input" placeholder="Example: Kalanki to Chabahil by bus" autocomplete="off" />
                    <button type="submit" id="chat-send" aria-label="Send"><i class="fa-solid fa-paper-plane"></i></button>
                </form>
            </div>
        </section>

        <section id="contribute" class="section section-reveal">
            <div class="section-head">
                <h2>Help improve Sawari</h2>
                <p>Submit missing stops, route updates, and local transit corrections.</p>
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
                    <textarea id="suggest-message" rows="5" maxlength="1000" placeholder="Share what needs correction" required></textarea>
                    <div class="count"><span id="suggest-chars">0</span> / 1000</div>

                    <button id="suggest-submit" type="submit"><i class="fa-solid fa-paper-plane"></i> Submit Suggestion</button>
                    <div id="suggest-feedback" class="suggest-feedback" style="display:none"></div>
                </form>

                <aside class="contrib-note">
                    <h3>Review flow</h3>
                    <ol>
                        <li>Suggestion is parsed and sent for admin review.</li>
                        <li>Admin verifies details against route data and field behavior.</li>
                        <li>Approved fixes update the transit dataset.</li>
                    </ol>
                    <p>Your name is optional. No email is required.</p>
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