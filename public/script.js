// Wkiti Frontend Interactive JavaScript
document.addEventListener('DOMContentLoaded', () => {
    console.log('[WKITI JS]: Interactive Application Loaded.');

    const counterEl = document.getElementById('click-counter');
    const btnIncrement = document.getElementById('btn-increment');
    const btnRefresh = document.getElementById('btn-refresh');

    function fetchStats() {
        fetch('/api/stats')
            .then(res => res.json())
            .then(data => {
                if (counterEl) counterEl.textContent = data.clicks;
            })
            .catch(err => console.error('[API Error]:', err));
    }

    if (btnIncrement) {
        btnIncrement.addEventListener('click', () => {
            fetch('/api/increment', { method: 'POST' })
                .then(res => res.json())
                .then(data => {
                    if (counterEl) counterEl.textContent = data.new_count;
                })
                .catch(err => console.error('[API Error]:', err));
        });
    }

    if (btnRefresh) {
        btnRefresh.addEventListener('click', fetchStats);
    }

    fetchStats();
});
