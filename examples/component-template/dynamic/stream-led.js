_component = zoid.create({
    tag: '%SELF_HOSTNAME%-stream-led',
    url: '%SELF_BASE_URL%/stream-led.html',
    dimensions: {
        width: '340px',
        height: '46px'
    }
});

if ("components" in window) {
    window.components["%SELF_HOSTNAME%"]["stream-led"] = _component
} else {
    window.component = _component
}
