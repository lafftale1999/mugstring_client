function goToSecondPage() {
    const name = document.getElementById("nameInput").value.trim();
    if (!name) {
        document.getElementById("nameInput").focus();
        return;
    }
    window.location.href = "/second.html?name=" + encodeURIComponent(name);
}

function showGreeting() {
    const params = new URLSearchParams(window.location.search);
    const name = params.get("name");
    const el = document.getElementById("greeting");
    if (el) {
        el.textContent = name ? "Welcome, " + name + "!" : "Welcome!";
    }
}
