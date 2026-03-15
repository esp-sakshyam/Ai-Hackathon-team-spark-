(function () {
    'use strict';

    /* ===== NAVBAR SCROLL EFFECT ===== */
    var navbar = document.querySelector('.navbar');
    var hero = document.getElementById('hero');
    if (navbar && hero) {
        var updateNavbar = function () {
            var heroBottom = hero.offsetHeight - 80;
            navbar.classList.toggle('scrolled', window.scrollY > heroBottom);
        };
        window.addEventListener('scroll', updateNavbar, { passive: true });
        updateNavbar();
    }

    /* ===== MOBILE MENU TOGGLE ===== */
    const mobileToggle = document.getElementById('mobile-toggle');
    const navLinks = document.getElementById('nav-links');
    if (mobileToggle && navLinks) {
        mobileToggle.addEventListener('click', function () {
            navLinks.classList.toggle('active');
            const icon = mobileToggle.querySelector('i');
            if (icon) {
                icon.className = navLinks.classList.contains('active')
                    ? 'fa-solid fa-xmark'
                    : 'fa-solid fa-bars';
            }
        });
        navLinks.querySelectorAll('a').forEach(function (link) {
            link.addEventListener('click', function () {
                navLinks.classList.remove('active');
                const icon = mobileToggle.querySelector('i');
                if (icon) { icon.className = 'fa-solid fa-bars'; }
            });
        });
    }

    /* ===== SCROLL REVEAL ===== */
    function initReveal() {
        const revealEls = document.querySelectorAll('.reveal');
        if (!revealEls.length) return;

        const observer = new IntersectionObserver(function (entries, obs) {
            entries.forEach(function (entry) {
                if (entry.isIntersecting) {
                    entry.target.classList.add('visible');
                    obs.unobserve(entry.target);
                }
            });
        }, { threshold: 0.1, rootMargin: '0px 0px -50px 0px' });

        revealEls.forEach(function (el, index) {
            el.style.transitionDelay = Math.min(index * 60, 360) + 'ms';
            observer.observe(el);
        });
    }

    /* ===== CHAT FEATURE ===== */
    const chatLog = document.getElementById('chat-log');
    const chatForm = document.getElementById('chat-form');
    const chatInput = document.getElementById('chat-input');
    const chatSend = document.getElementById('chat-send');

    const SYSTEM_PROMPT = "You are Sawari, a practical assistant for Kathmandu Valley public transit.\n\nYou can help with:\n- Bus and micro routes around Kathmandu, Lalitpur, and Bhaktapur\n- Common stop names and transfer suggestions\n- Fare guidance in Nepali context\n\nGuidelines:\n- Be concise and direct\n- If unsure, say so and suggest opening the full Sawari navigator\n- Keep most responses under 140 words";

    const messages = [{ role: 'system', content: SYSTEM_PROMPT }];

    function escapeHtml(str) {
        var div = document.createElement('div');
        div.textContent = str;
        return div.innerHTML;
    }

    function addMessage(role, text) {
        if (!chatLog) return;
        var msg = document.createElement('div');
        msg.className = 'chat-msg ' + role;

        var avatar = document.createElement('div');
        avatar.className = 'chat-avatar';
        avatar.innerHTML = role === 'assistant'
            ? '<i class="fa-solid fa-bus"></i>'
            : '<i class="fa-solid fa-user"></i>';

        var bubble = document.createElement('div');
        bubble.className = 'chat-bubble';
        bubble.innerHTML = escapeHtml(text).replace(/\n/g, '<br>');

        msg.appendChild(avatar);
        msg.appendChild(bubble);
        chatLog.appendChild(msg);
        chatLog.scrollTop = chatLog.scrollHeight;
    }

    function addTypingIndicator() {
        if (!chatLog) return;
        var row = document.createElement('div');
        row.className = 'chat-msg assistant';
        row.id = 'typing-indicator';

        var avatar = document.createElement('div');
        avatar.className = 'chat-avatar';
        avatar.innerHTML = '<i class="fa-solid fa-bus"></i>';

        var bubble = document.createElement('div');
        bubble.className = 'chat-bubble chat-typing';
        bubble.innerHTML = '<span></span><span></span><span></span>';

        row.appendChild(avatar);
        row.appendChild(bubble);
        chatLog.appendChild(row);
        chatLog.scrollTop = chatLog.scrollHeight;
    }

    function removeTypingIndicator() {
        var typing = document.getElementById('typing-indicator');
        if (typing) typing.remove();
    }

    async function sendMessage(userText) {
        addMessage('user', userText);
        messages.push({ role: 'user', content: userText });

        chatInput.value = '';
        chatInput.disabled = true;
        chatSend.disabled = true;
        addTypingIndicator();

        try {
            var res = await fetch('https://api.groq.com/openai/v1/chat/completions', {
                method: 'POST',
                headers: {
                    'Content-Type': 'application/json',
                    Authorization: 'Bearer ' + GROQ_API_KEY
                },
                body: JSON.stringify({
                    model: 'llama-3.3-70b-versatile',
                    messages: messages.slice(-10),
                    max_tokens: 420,
                    temperature: 0.6
                })
            });

            removeTypingIndicator();
            if (!res.ok) {
                var errText = await res.text();
                throw new Error('API ' + res.status + ': ' + errText);
            }

            var data = await res.json();
            var reply = data.choices && data.choices[0] && data.choices[0].message
                ? data.choices[0].message.content
                : 'I could not generate an answer right now. Please try again.';

            messages.push({ role: 'assistant', content: reply });
            addMessage('assistant', reply);
        } catch (err) {
            removeTypingIndicator();
            addMessage('assistant', 'Chat is currently unavailable. Please open the navigator for route planning.');
            console.error(err);
        } finally {
            chatInput.disabled = false;
            chatSend.disabled = false;
            chatInput.focus();
        }
    }

    if (chatForm && chatInput) {
        chatForm.addEventListener('submit', function (event) {
            event.preventDefault();
            var text = chatInput.value.trim();
            if (!text) return;
            sendMessage(text);
        });
    }

    /* ===== EXAMPLE QUERY CHIPS ===== */
    document.querySelectorAll('.ask-example').forEach(function (chip) {
        chip.addEventListener('click', function () {
            var query = this.getAttribute('data-query');
            if (query && chatInput) {
                chatInput.value = query;
                chatInput.focus();
                sendMessage(query);
            }
        });
    });

    /* ===== SUGGESTION FORM ===== */
    var suggestForm = document.getElementById('suggest-form');
    var suggestMessage = document.getElementById('suggest-message');
    var suggestChars = document.getElementById('suggest-chars');
    var suggestSubmit = document.getElementById('suggest-submit');
    var suggestFeedback = document.getElementById('suggest-feedback');

    if (suggestMessage && suggestChars) {
        suggestMessage.addEventListener('input', function () {
            suggestChars.textContent = String(this.value.length);
        });
    }

    if (suggestForm && suggestMessage && suggestSubmit && suggestFeedback) {
        suggestForm.addEventListener('submit', async function (event) {
            event.preventDefault();

            var nameInput = document.getElementById('suggest-name');
            var categoryInput = document.getElementById('suggest-category');

            var name = nameInput ? nameInput.value.trim() || 'Anonymous' : 'Anonymous';
            var category = categoryInput ? categoryInput.value : 'general';
            var message = suggestMessage.value.trim();

            if (!message || message.length < 10) {
                showSuggestFeedback('Please write at least 10 characters.', 'error');
                return;
            }

            suggestSubmit.disabled = true;
            suggestSubmit.innerHTML = '<i class="fa-solid fa-spinner fa-spin"></i> Submitting';

            try {
                var res = await fetch('app/backend/handlers/suggestions.php', {
                    method: 'POST',
                    headers: { 'Content-Type': 'application/json' },
                    body: JSON.stringify({ name: name, category: category, message: message })
                });

                var data = await res.json();
                if (!res.ok) {
                    throw new Error(data.error || 'Submission failed');
                }

                suggestForm.reset();
                suggestChars.textContent = '0';
                var taskNote = data.task
                    ? ' A task was extracted for admin review.'
                    : ' Your input has been queued for manual review.';
                showSuggestFeedback('Thank you for helping improve Sawari.' + taskNote, 'success');
            } catch (err) {
                showSuggestFeedback(err.message || 'Something went wrong. Please try again.', 'error');
                console.error(err);
            } finally {
                suggestSubmit.disabled = false;
                suggestSubmit.innerHTML = '<i class="fa-solid fa-paper-plane"></i> Submit Suggestion';
            }
        });
    }

    function showSuggestFeedback(message, type) {
        if (!suggestFeedback) return;
        suggestFeedback.textContent = message;
        suggestFeedback.className = 'suggest-feedback ' + type;
        suggestFeedback.style.display = 'block';
        setTimeout(function () {
            suggestFeedback.style.display = 'none';
        }, 8000);
    }

    /* ===== INIT ===== */
    initReveal();
})();
