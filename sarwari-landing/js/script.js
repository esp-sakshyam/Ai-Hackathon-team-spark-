/* ==================================================================
   SAWARI LANDING — Script v4 (Redesign)
   Vanilla JS · IntersectionObserver · Smooth interactions
   ================================================================== */
;(function () {
    'use strict';

    var raf = window.requestAnimationFrame || function (cb) { setTimeout(cb, 16); };

    /* ── 1. Sticky Navbar ── */
    var navbar = document.getElementById('navbar');
    function updateNavbar() {
        if (navbar) navbar.classList.toggle('scrolled', window.scrollY > 40);
    }

    /* ── 2. Active Nav Link ── */
    var navLinks = document.querySelectorAll('.navbar__link');
    var sectionAnchors = [];
    navLinks.forEach(function (link) {
        var href = link.getAttribute('href');
        if (href && href.charAt(0) === '#') {
            var target = document.querySelector(href);
            if (target) sectionAnchors.push({ el: target, link: link });
        }
    });

    function updateActiveNav() {
        var scrollPos = window.scrollY + 160;
        var current = null;
        for (var i = 0; i < sectionAnchors.length; i++) {
            if (sectionAnchors[i].el.offsetTop <= scrollPos) current = sectionAnchors[i];
        }
        navLinks.forEach(function (l) { l.classList.remove('active'); });
        if (current) current.link.classList.add('active');
    }

    /* ── 3. Back to Top ── */
    var backToTop = document.getElementById('backToTop');
    function updateBackToTop() {
        if (backToTop) backToTop.classList.toggle('visible', window.scrollY > 600);
    }
    if (backToTop) {
        backToTop.addEventListener('click', function () {
            window.scrollTo({ top: 0, behavior: 'smooth' });
        });
    }

    /* ── Combined Scroll (rAF throttled) ── */
    var ticking = false;
    function onScroll() {
        if (!ticking) {
            raf(function () {
                updateNavbar();
                updateActiveNav();
                updateBackToTop();
                ticking = false;
            });
            ticking = true;
        }
    }
    window.addEventListener('scroll', onScroll, { passive: true });
    updateNavbar();
    updateActiveNav();

    /* ── 4. Mobile Menu ── */
    var hamburger = document.getElementById('hamburger');
    var navLinksContainer = document.getElementById('navLinks');
    var mobileOverlay = document.getElementById('mobileOverlay');

    function toggleMobile() {
        if (!hamburger || !navLinksContainer) return;
        var isOpen = hamburger.classList.toggle('active');
        navLinksContainer.classList.toggle('mobile-active');
        if (mobileOverlay) mobileOverlay.classList.toggle('active');
        hamburger.setAttribute('aria-expanded', isOpen);
        document.body.style.overflow = isOpen ? 'hidden' : '';
    }
    function closeMobile() {
        if (!hamburger || !navLinksContainer) return;
        hamburger.classList.remove('active');
        navLinksContainer.classList.remove('mobile-active');
        if (mobileOverlay) mobileOverlay.classList.remove('active');
        hamburger.setAttribute('aria-expanded', 'false');
        document.body.style.overflow = '';
    }
    if (hamburger) hamburger.addEventListener('click', toggleMobile);
    if (mobileOverlay) mobileOverlay.addEventListener('click', closeMobile);
    navLinks.forEach(function (link) {
        link.addEventListener('click', function () {
            if (navLinksContainer && navLinksContainer.classList.contains('mobile-active')) closeMobile();
        });
    });

    /* ── 5. Smooth Scroll ── */
    document.querySelectorAll('a[href^="#"]').forEach(function (anchor) {
        anchor.addEventListener('click', function (e) {
            var targetId = this.getAttribute('href');
            if (targetId === '#') return;
            var target = document.querySelector(targetId);
            if (!target) return;
            e.preventDefault();
            var navH = navbar ? navbar.offsetHeight : 0;
            var pos = target.getBoundingClientRect().top + window.scrollY - navH - 16;
            window.scrollTo({ top: pos, behavior: 'smooth' });
        });
    });

    /* ── 6. Scroll Reveal (IntersectionObserver) ── */
    var animEls = document.querySelectorAll('[data-animate]');
    if ('IntersectionObserver' in window) {
        var revealObs = new IntersectionObserver(function (entries) {
            entries.forEach(function (entry) {
                if (entry.isIntersecting) {
                    entry.target.classList.add('visible');
                    revealObs.unobserve(entry.target);
                }
            });
        }, { threshold: 0.1, rootMargin: '0px 0px -40px 0px' });
        animEls.forEach(function (el) { revealObs.observe(el); });
    } else {
        animEls.forEach(function (el) { el.classList.add('visible'); });
    }

    /* ── 7. Animated Counters ── */
    var counterEls = document.querySelectorAll('[data-count]');
    function animateCounter(el) {
        var target = parseInt(el.getAttribute('data-count'), 10);
        if (isNaN(target)) return;
        var duration = 2000;
        var start = performance.now();
        function tick(now) {
            var elapsed = now - start;
            var progress = Math.min(elapsed / duration, 1);
            var eased = 1 - Math.pow(1 - progress, 4);
            el.textContent = Math.round(eased * target);
            if (progress < 1) raf(tick);
            else el.textContent = target;
        }
        raf(tick);
    }
    if ('IntersectionObserver' in window && counterEls.length) {
        var counterObs = new IntersectionObserver(function (entries) {
            entries.forEach(function (entry) {
                if (entry.isIntersecting) {
                    animateCounter(entry.target);
                    counterObs.unobserve(entry.target);
                }
            });
        }, { threshold: 0.5 });
        counterEls.forEach(function (el) { counterObs.observe(el); });
    }

    /* ── 8. Suggestion Form ── */
    var form = document.getElementById('suggestionForm');
    if (form) {
        form.addEventListener('submit', function (e) {
            e.preventDefault();
            var btn = form.querySelector('.feedback-submit');
            if (!btn) return;
            var original = btn.innerHTML;
            btn.innerHTML = '<svg width="16" height="16" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2.5" stroke-linecap="round" stroke-linejoin="round"><polyline points="20 6 9 17 4 12"></polyline></svg> Thank You!';
            btn.style.pointerEvents = 'none';
            btn.style.opacity = '.7';
            setTimeout(function () {
                form.reset();
                btn.innerHTML = original;
                btn.style.pointerEvents = '';
                btn.style.opacity = '';
            }, 3000);
        });
    }

})();
