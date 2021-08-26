window.addEventListener('load', function() {
    if (typeof main !== 'undefined' && typeof main === 'function') {
        main();
    }
});
function main() { console.log("Hello World!"); }