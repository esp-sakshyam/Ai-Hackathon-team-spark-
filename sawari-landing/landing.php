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
    <title>Sawari &mdash; Kathmandu Valley Transit</title>
    <meta name="description"
        content="Navigate Kathmandu Valley public transit with practical route guidance, fare estimates, and nearby stop discovery." />
    <link rel="preconnect" href="https://fonts.googleapis.com" />
    <link rel="preconnect" href="https://fonts.gstatic.com" crossorigin />
    <link
        href="https://fonts.googleapis.com/css2?family=DM+Serif+Display&family=Inter:wght@400;500;600;700;800&display=swap"
        rel="stylesheet" />
    <link rel="stylesheet" href="https://cdnjs.cloudflare.com/ajax/libs/font-awesome/6.5.1/css/all.min.css" />
    <link rel="stylesheet" href="landing.css" />
</head>

<body>
    <!-- ==================== NAVBAR ==================== -->
    <header class="navbar" id="top">
        <div class="navbar-inner">
            <a href="#top" class="logo">
                <i class="fa-solid fa-bus-simple"></i>
                <span>Sawari</span>
            </a>
            <nav class="nav-links" id="nav-links">
                <a href="#about">About</a>
                <a href="#how">How it works</a>
                <a href="#features">Features</a>
                <a href="#gallery">Gallery</a>
                <a href="#ask">Ask AI</a>
                <a href="#contribute">Contribute</a>
            </nav>
            <a href="../app/index.php" class="btn-cta-nav">Open Navigator <i class="fa-solid fa-arrow-right"></i></a>
            <button class="mobile-toggle" id="mobile-toggle" aria-label="Toggle menu">
                <i class="fa-solid fa-bars"></i>
            </button>
        </div>
    </header>

    <main>
        <!-- ==================== HERO ==================== -->
        <section class="hero reveal" id="hero">
            <div class="hero-bg">
                <img src="gallery/landing.jpg" alt="Kathmandu transit" />
            </div>
            <div class="hero-overlay"></div>
            <div class="hero-content">
                <div class="hero-text">
                    <div class="hero-badge">
                        <i class="fa-solid fa-location-dot"></i> Kathmandu Valley Transit Companion
                    </div>
                    <h1>Local transit,<br><span class="text-gradient">beautifully mapped.</span></h1>
                    <p class="hero-desc">
                        Plan your bus journey with practical route guidance, transfer intelligence, fare estimates,
                        and nearby stop discovery built around how Kathmandu actually moves.
                    </p>
                    <div class="hero-actions">
                        <a class="btn-primary" href="../app/index.php">
                            <i class="fa-solid fa-route"></i> Plan a Journey
                        </a>
                        <a class="btn-outline" href="#how">
                            Explore the System <i class="fa-solid fa-chevron-down"></i>
                        </a>
                    </div>
                    <div class="hero-tags">
                        <span><i class="fa-solid fa-route"></i> Route planner</span>
                        <span><i class="fa-solid fa-bus"></i> Local operators</span>
                        <span><i class="fa-solid fa-map-location-dot"></i> Nearby stops</span>
                    </div>
                </div>
                <div class="hero-mosaic">
                    <div class="mosaic-large">
                        <img src="gallery/sajha-bus-EV.jpg" alt="Electric Sajha bus" loading="lazy" />
                    </div>
                    <div class="mosaic-stack">
                        <img src="gallery/hiace.webp" alt="Hiace microbus" loading="lazy" />
                        <img src="gallery/mahanagar.jpg" alt="Mahanagar bus" loading="lazy" />
                    </div>
                    <div class="mosaic-wide">
                        <img src="gallery/digo-yatayat.jpg" alt="Digo yatayat" loading="lazy" />
                    </div>
                </div>
            </div>
            <div class="hero-scroll-hint">
                <i class="fa-solid fa-chevron-down"></i>
            </div>
        </section>

        <!-- ==================== STATS BAR ==================== -->
        <section class="stats-bar reveal">
            <div class="stat">
                <img src="gallery/sajha.jpg" alt="Sajha" class="stat-img" loading="lazy" />
                <div>
                    <strong>50+</strong>
                    <span>Transit Routes</span>
                </div>
            </div>
            <div class="stat">
                <img src="gallery/mayur.jpg" alt="Mayur" class="stat-img" loading="lazy" />
                <div>
                    <strong>500+</strong>
                    <span>Bus Stops</span>
                </div>
            </div>
            <div class="stat">
                <img src="gallery/tempo.jpg" alt="Tempo" class="stat-img" loading="lazy" />
                <div>
                    <strong>3</strong>
                    <span>Districts Covered</span>
                </div>
            </div>
            <div class="stat">
                <img src="gallery/nilo-micro.jpg" alt="Micro" class="stat-img" loading="lazy" />
                <div>
                    <strong>AI</strong>
                    <span>Powered Search</span>
                </div>
            </div>
        </section>

        <!-- ==================== ABOUT ==================== -->
        <section id="about" class="section reveal">
            <div class="section-header">
                <span class="section-tag">About Sawari</span>
                <h2>What Sawari does</h2>
                <p>All core capabilities, presented with clarity and purpose.</p>
            </div>
            <div class="about-cards">
                <article class="about-card">
                    <div class="about-card-img">
                        <img src="gallery/routes.jpg" alt="Kathmandu routes" loading="lazy" />
                    </div>
                    <div class="about-card-body">
                        <div class="about-icon"><i class="fa-solid fa-map"></i></div>
                        <h3>Built for Kathmandu routes</h3>
                        <p>
                            Sawari models local route behavior and common stop names across Kathmandu, Lalitpur,
                            and Bhaktapur, so commuters can search naturally.
                        </p>
                    </div>
                </article>
                <article class="about-card">
                    <div class="about-card-img">
                        <img src="gallery/high-tech-buses.jpg" alt="Modern buses" loading="lazy" />
                    </div>
                    <div class="about-card-body">
                        <div class="about-icon"><i class="fa-solid fa-globe"></i></div>
                        <h3>No app required</h3>
                        <p>
                            Everything works from browser only. Open the navigator, enter points, and start moving.
                        </p>
                    </div>
                </article>
                <article class="about-card">
                    <div class="about-card-img">
                        <img src="gallery/nepal.jpg" alt="Nepal transit" loading="lazy" />
                    </div>
                    <div class="about-card-body">
                        <div class="about-icon"><i class="fa-solid fa-users"></i></div>
                        <h3>Community-corrected data</h3>
                        <p>
                            Riders can submit missing stops and route corrections that flow to admin verification.
                        </p>
                    </div>
                </article>
            </div>

            <div class="photo-strip">
                <div class="photo-strip-track">
                    <img src="gallery/image1.jpg" alt="Transit photo 1" loading="lazy" />
                    <img src="gallery/image2.jpg" alt="Transit photo 2" loading="lazy" />
                    <img src="gallery/image3.jpg" alt="Transit photo 3" loading="lazy" />
                    <img src="gallery/image4.jpg" alt="Transit photo 4" loading="lazy" />
                    <img src="gallery/image5.jpg" alt="Transit photo 5" loading="lazy" />
                    <img src="gallery/image6.jpg" alt="Transit photo 6" loading="lazy" />
                    <img src="gallery/image7.jpg" alt="Transit photo 7" loading="lazy" />
                    <img src="gallery/image1.jpg" alt="Transit photo 1" loading="lazy" aria-hidden="true" />
                    <img src="gallery/image2.jpg" alt="Transit photo 2" loading="lazy" aria-hidden="true" />
                    <img src="gallery/image3.jpg" alt="Transit photo 3" loading="lazy" aria-hidden="true" />
                </div>
            </div>
        </section>

        <!-- ==================== HOW IT WORKS ==================== -->
        <section id="how" class="section section-dark reveal">
            <div class="section-header section-header-light">
                <span class="section-tag">Process</span>
                <h2>How it works</h2>
                <p>Four simple steps to navigate Kathmandu Valley transit.</p>
            </div>
            <div class="steps-grid">
                <article class="step-card">
                    <div class="step-number">01</div>
                    <div class="step-visual">
                        <img src="gallery/SajhaBus_20220707134627.jpg" alt="Enter destination" loading="lazy" />
                    </div>
                    <h3>Enter origin and destination</h3>
                    <p>Use natural language or map points to start your journey query.</p>
                </article>
                <article class="step-card">
                    <div class="step-number">02</div>
                    <div class="step-visual">
                        <img src="gallery/sajha-yatayat-kathmandu-narayangadh-shuttle-service-1758693554.jpg"
                            alt="See routes" loading="lazy" />
                    </div>
                    <h3>See direct and transfer routes</h3>
                    <p>Sawari checks nearby stop networks and suggests practical combinations.</p>
                </article>
                <article class="step-card">
                    <div class="step-number">03</div>
                    <div class="step-visual">
                        <img src="gallery/samakhusi-yatayat-1729569860.jpg" alt="Check fare" loading="lazy" />
                    </div>
                    <h3>Check fare and walking legs</h3>
                    <p>Fare estimates and short walk segments are shown with clear trip structure.</p>
                </article>
                <article class="step-card">
                    <div class="step-number">04</div>
                    <div class="step-visual">
                        <img src="gallery/sajha1.jpg" alt="Track buses" loading="lazy" />
                    </div>
                    <h3>Track nearby buses when available</h3>
                    <p>Live bus updates support better timing and boarding choices.</p>
                </article>
            </div>
        </section>

        <!-- ==================== FEATURES ==================== -->
        <section id="features" class="section reveal">
            <div class="section-header">
                <span class="section-tag">Features</span>
                <h2>Everything included</h2>
                <p>A complete toolkit for navigating public transit.</p>
            </div>
            <div class="features-grid">
                <article class="feature-card">
                    <div class="feature-icon"><i class="fa-solid fa-route"></i></div>
                    <h3>Route Planning</h3>
                    <p>Direct and transfer route discovery using stop-level matching.</p>
                    <img src="gallery/mahanagar-yatayat.png" alt="Route planning" class="feature-bg-img" loading="lazy" />
                </article>
                <article class="feature-card">
                    <div class="feature-icon"><i class="fa-solid fa-ticket"></i></div>
                    <h3>Fare Estimation</h3>
                    <p>Fare approximation aligned with local tariff logic.</p>
                    <img src="gallery/manangar-yatayat.jpg" alt="Fare estimation" class="feature-bg-img" loading="lazy" />
                </article>
                <article class="feature-card feature-card-accent">
                    <div class="feature-icon"><i class="fa-solid fa-wand-magic-sparkles"></i></div>
                    <h3>Natural Language</h3>
                    <p>Supports practical everyday phrasing from users.</p>
                    <img src="gallery/IMG-1.jpeg" alt="Natural language" class="feature-bg-img" loading="lazy" />
                </article>
                <article class="feature-card">
                    <div class="feature-icon"><i class="fa-solid fa-location-arrow"></i></div>
                    <h3>GPS and Nearby Stops</h3>
                    <p>Locates surrounding stops and available passing routes.</p>
                    <img src="gallery/mayur_yatayat_XWhLImWBJm.jpg" alt="GPS stops" class="feature-bg-img" loading="lazy" />
                </article>
                <article class="feature-card feature-card-accent">
                    <div class="feature-icon"><i class="fa-solid fa-leaf"></i></div>
                    <h3>Carbon Comparison</h3>
                    <p>Displays approximate CO2 savings for public transit use.</p>
                    <img src="gallery/39089002_214055306121394_3987346834723962880_n_mmZU29H7GE.jpg" alt="Carbon comparison" class="feature-bg-img" loading="lazy" />
                </article>
                <article class="feature-card">
                    <div class="feature-icon"><i class="fa-solid fa-circle-half-stroke"></i></div>
                    <h3>Dark and Light Themes</h3>
                    <p>Theme options for readability in different lighting conditions.</p>
                    <img src="gallery/81038617_2710162452340155_22013957954863104_n.jpg" alt="Themes" class="feature-bg-img" loading="lazy" />
                </article>
            </div>
        </section>

        <!-- ==================== GALLERY ==================== -->
        <section id="gallery" class="section reveal">
            <div class="section-header">
                <span class="section-tag">Gallery</span>
                <h2>Gallery of transit life</h2>
                <p>A visual journey through Kathmandu Valley's public transit system.</p>
            </div>
            <div class="gallery-masonry">
                <div class="gallery-col">
                    <img src="gallery/1594641445.jpg" alt="Kathmandu road transit" loading="lazy" />
                    <img src="gallery/473749570_1355280112583010_593412764911101796_n.jpg" alt="Public bus" loading="lazy" />
                    <img src="gallery/high-tech-buses.jpg" alt="High tech buses" loading="lazy" />
                    <img src="gallery/sajha.jpg" alt="Sajha bus" loading="lazy" />
                    <img src="gallery/samakhusi-yatayat-1729569860.jpg" alt="Samakhusi route" loading="lazy" />
                </div>
                <div class="gallery-col">
                    <img src="gallery/302216934_482557370551393_6475660738973236802_n.jpg" alt="Bus service" loading="lazy" />
                    <img src="gallery/IMG-1.jpeg" alt="Transit Kathmandu" loading="lazy" />
                    <img src="gallery/mahanagar-yatayat.png" alt="Mahanagar yatayat" loading="lazy" />
                    <img src="gallery/nepal.jpg" alt="Nepal transit" loading="lazy" />
                    <img src="gallery/routes.jpg" alt="Route board" loading="lazy" />
                </div>
                <div class="gallery-col">
                    <img src="gallery/39089002_214055306121394_3987346834723962880_n_mmZU29H7GE.jpg" alt="Older bus" loading="lazy" />
                    <img src="gallery/481820024_656265493582501_1243983797300762177_n.jpg" alt="Modern local bus" loading="lazy" />
                    <img src="gallery/mayur.jpg" alt="Mayur bus" loading="lazy" />
                    <img src="gallery/nilo-micro.jpg" alt="Blue microbus" loading="lazy" />
                    <img src="gallery/tempo.jpg" alt="Tempo transport" loading="lazy" />
                </div>
                <div class="gallery-col">
                    <img src="gallery/473783154_1062778689195922_6904992953434292580_n.jpg" alt="Transport route" loading="lazy" />
                    <img src="gallery/81038617_2710162452340155_22013957954863104_n.jpg" alt="Transit community" loading="lazy" />
                    <img src="gallery/manangar-yatayat.jpg" alt="Yatayat bus" loading="lazy" />
                    <img src="gallery/sajha1.jpg" alt="Sajha route" loading="lazy" />
                    <img src="gallery/SajhaBus_20220707134627.jpg" alt="Sajha bus" loading="lazy" />
                    <img src="gallery/sajha-yatayat-kathmandu-narayangadh-shuttle-service-1758693554.jpg" alt="Sajha shuttle" loading="lazy" />
                    <img src="gallery/mayur_yatayat_XWhLImWBJm.jpg" alt="Mayur yatayat" loading="lazy" />
                </div>
            </div>
        </section>

        <!-- ==================== ASK SAWARI ==================== -->
        <section id="ask" class="section section-dark reveal">
            <div class="section-header section-header-light">
                <span class="section-tag">AI Assistant</span>
                <h2>Ask Sawari</h2>
                <p>Transit questions answered in simple language.</p>
            </div>
            <div class="ask-layout">
                <div class="ask-info">
                    <div class="ask-info-card">
                        <img src="gallery/sajha-bus-EV.jpg" alt="Sajha EV bus" loading="lazy" />
                        <div class="ask-info-overlay">
                            <h3>AI-Powered Transit Help</h3>
                            <p>Ask about routes, fares, or nearby stops and get instant answers powered by AI.</p>
                        </div>
                    </div>
                    <div class="ask-examples">
                        <span class="ask-example" data-query="How do I get from Kalanki to Chabahil?">Kalanki to Chabahil</span>
                        <span class="ask-example" data-query="What is the bus fare from Ratnapark to Lagankhel?">Ratnapark fare</span>
                        <span class="ask-example" data-query="Which buses pass through Balaju?">Buses through Balaju</span>
                        <span class="ask-example" data-query="How to reach Bhaktapur from Koteshwor?">Koteshwor to Bhaktapur</span>
                    </div>
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
                        <input type="text" id="chat-input" placeholder="Example: Kalanki to Chabahil by bus"
                            autocomplete="off" />
                        <button type="submit" id="chat-send" aria-label="Send"><i
                                class="fa-solid fa-paper-plane"></i></button>
                    </form>
                </div>
            </div>
        </section>

        <!-- ==================== CONTRIBUTE ==================== -->
        <section id="contribute" class="section reveal">
            <div class="section-header">
                <span class="section-tag">Community</span>
                <h2>Help improve Sawari</h2>
                <p>Submit missing stops, route updates, and local transit corrections.</p>
            </div>
            <div class="contribute-layout">
                <form id="suggest-form" class="suggest-form">
                    <label for="suggest-name">Your Name <span class="label-hint">(optional)</span></label>
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
                    <textarea id="suggest-message" rows="5" maxlength="1000"
                        placeholder="Share what needs correction" required></textarea>
                    <div class="char-count"><span id="suggest-chars">0</span> / 1000</div>

                    <button id="suggest-submit" type="submit"><i class="fa-solid fa-paper-plane"></i> Submit
                        Suggestion</button>
                    <div id="suggest-feedback" class="suggest-feedback" style="display:none"></div>
                </form>

                <aside class="contribute-aside">
                    <div class="contribute-aside-top">
                        <img src="gallery/mahanagar.jpg" alt="Bus in Kathmandu" loading="lazy" />
                    </div>
                    <div class="contribute-aside-body">
                        <h3>Review flow</h3>
                        <ol>
                            <li>Suggestion is parsed and sent for admin review.</li>
                            <li>Admin verifies details against route data and field behavior.</li>
                            <li>Approved fixes update the transit dataset.</li>
                        </ol>
                        <p>Your name is optional. No email is required.</p>
                        <div class="contribute-links">
                            <a href="../app/index.php" class="btn-sm">
                                <i class="fa-solid fa-compass"></i> Navigator
                            </a>
                            <a href="../app/admin/" class="btn-sm btn-sm-outline">
                                <i class="fa-solid fa-shield-halved"></i> Admin
                            </a>
                        </div>
                    </div>
                </aside>
            </div>
        </section>
    </main>

    <!-- ==================== FOOTER ==================== -->
    <footer class="footer">
        <div class="footer-inner">
            <div class="footer-brand">
                <span class="logo"><i class="fa-solid fa-bus-simple"></i> Sawari</span>
                <p>Navigating Kathmandu Valley, together.</p>
            </div>
            <div class="footer-links">
                <a href="#about">About</a>
                <a href="#how">How it works</a>
                <a href="#features">Features</a>
                <a href="#gallery">Gallery</a>
                <a href="#ask">Ask AI</a>
                <a href="#contribute">Contribute</a>
            </div>
            <div class="footer-bottom">
                <p>&copy; <?= date('Y') ?> Sawari by Team Spark. All rights reserved.</p>
            </div>
        </div>
    </footer>

    <script>const GROQ_API_KEY = <?= json_encode($groqApiKey) ?>;</script>
    <script src="landing.js"></script>
</body>

</html>
