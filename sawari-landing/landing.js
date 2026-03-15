(function () {
    'use strict';

    const chatLog = document.getElementById('chat-log');
    const chatForm = document.getElementById('chat-form');
    const chatInput = document.getElementById('chat-input');
    const chatSend = document.getElementById('chat-send');

    const SYSTEM_PROMPT = `You are Sawari, a practical assistant for Kathmandu Valley public transit.

You can help with:
- Bus and micro routes around Kathmandu, Lalitpur, and Bhaktapur
- Common stop names and transfer suggestions
- Fare guidance in Nepali context

Guidelines:
- Be concise and direct
- If unsure, say so and suggest opening the full Sawari navigator
- Keep most responses under 140 words`;

    const messages = [{ role: 'system', content: SYSTEM_PROMPT }];

    function escapeHtml(str) {
        const div = document.createElement('div');
        div.textContent = str;
        return div.innerHTML;
    }

    function addMessage(role, text) {
        if (!chatLog) {
            return;
        }
        const msg = document.createElement('div');
        msg.className = `chat-msg ${role}`;

        const avatar = document.createElement('div');
        avatar.className = 'chat-avatar';
        avatar.innerHTML = role === 'assistant'
            ? '<i class="fa-solid fa-bus"></i>'
            : '<i class="fa-solid fa-user"></i>';

        const bubble = document.createElement('div');
        bubble.className = 'chat-bubble';
        bubble.innerHTML = escapeHtml(text).replace(/\n/g, '<br>');

        msg.appendChild(avatar);
        msg.appendChild(bubble);
        chatLog.appendChild(msg);
        chatLog.scrollTop = chatLog.scrollHeight;
    }

    function addTypingIndicator() {
        if (!chatLog) {
            return;
        }
        const row = document.createElement('div');
        row.className = 'chat-msg assistant';
        row.id = 'typing-indicator';

        const avatar = document.createElement('div');
        avatar.className = 'chat-avatar';
        avatar.innerHTML = '<i class="fa-solid fa-bus"></i>';

        const bubble = document.createElement('div');
        bubble.className = 'chat-bubble chat-typing';
        bubble.innerHTML = '<span></span><span></span><span></span>';

        row.appendChild(avatar);
        row.appendChild(bubble);
        chatLog.appendChild(row);
        chatLog.scrollTop = chatLog.scrollHeight;
    }

    function removeTypingIndicator() {
        const typing = document.getElementById('typing-indicator');
        if (typing) {
            typing.remove();
        }
    }

    async function sendMessage(userText) {
        addMessage('user', userText);
        messages.push({ role: 'user', content: userText });

        chatInput.value = '';
        chatInput.disabled = true;
        chatSend.disabled = true;
        addTypingIndicator();

        try {
            const res = await fetch('https://api.groq.com/openai/v1/chat/completions', {
                method: 'POST',
                headers: {
                    'Content-Type': 'application/json',
                    Authorization: `Bearer ${GROQ_API_KEY}`
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
                const text = await res.text();
                throw new Error(`API ${res.status}: ${text}`);
            }

            const data = await res.json();
            const reply = data.choices && data.choices[0] && data.choices[0].message
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
            const text = chatInput.value.trim();
            if (!text) {
                return;
            }
            sendMessage(text);
        });
    }

    const suggestForm = document.getElementById('suggest-form');
    const suggestMessage = document.getElementById('suggest-message');
    const suggestChars = document.getElementById('suggest-chars');
    const suggestSubmit = document.getElementById('suggest-submit');
    const suggestFeedback = document.getElementById('suggest-feedback');

    if (suggestMessage && suggestChars) {
        suggestMessage.addEventListener('input', function () {
            suggestChars.textContent = String(this.value.length);
        });
    }

    if (suggestForm && suggestMessage && suggestSubmit && suggestFeedback) {
        suggestForm.addEventListener('submit', async function (event) {
            event.preventDefault();

            const nameInput = document.getElementById('suggest-name');
            const categoryInput = document.getElementById('suggest-category');

            const name = nameInput ? nameInput.value.trim() || 'Anonymous' : 'Anonymous';
            const category = categoryInput ? categoryInput.value : 'general';
            const message = suggestMessage.value.trim();

            if (!message || message.length < 10) {
                showSuggestFeedback('Please write at least 10 characters.', 'error');
                return;
            }

            suggestSubmit.disabled = true;
            suggestSubmit.innerHTML = '<i class="fa-solid fa-spinner fa-spin"></i> Submitting';

            try {
                const res = await fetch('../app/backend/handlers/suggestions.php', {
                    method: 'POST',
                    headers: { 'Content-Type': 'application/json' },
                    body: JSON.stringify({ name, category, message })
                });

                const data = await res.json();
                if (!res.ok) {
                    throw new Error(data.error || 'Submission failed');
                }

                suggestForm.reset();
                suggestChars.textContent = '0';
                const taskNote = data.task
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
        if (!suggestFeedback) {
            return;
        }
        suggestFeedback.textContent = message;
        suggestFeedback.className = `suggest-feedback ${type}`;
        suggestFeedback.style.display = 'block';
        setTimeout(function () {
            suggestFeedback.style.display = 'none';
        }, 8000);
    }
})();