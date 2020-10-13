_component = zoid.create({
    tag: '%SELF_HOSTNAME%-stream',
    url: '%SELF_BASE_URL%/stream.html',
    dimensions: {
        width: '340px',
        height: '260px'
    }
});

if ("components" in window) {
    window.components["%SELF_HOSTNAME%"]["stream"] = _component
} else {
    window.component = _component
}
