

let gl = null;
let viewport = { x: 0, y: 0, width: 0, height: 0 };
    
function create_3d_canvas() {
    const canvas = document.createElement("canvas");
    canvas.className = "main-canvas";
    document.body.append(canvas);

    gl = (canvas.getContext("webgl") ||
          canvas.getContext("experimental-webgl"));
    
    if (gl && gl instanceof WebGLRenderingContext) {
        create_notification("Congratulations! Your browser supports WebGL.");
        document.documentElement.style.backgroundColor = "transparent";
    } else {
        create_notification("Your browser or device may not support WebGL.", "m-error");
    }

    viewport.width = gl.drawingBufferWidth;
    viewport.height = gl.drawingBufferWidth;
    gl.viewport(viewport.x, viewport.y, viewport.width, viewport.height);

    gl.clearColor(0, 0.5, 1.0, 1.0);
    gl.clear(gl.COLOR_BUFFER_BIT);
}
