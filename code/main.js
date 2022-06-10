
function create_notification(html, modifier="m-info") {
    const target = document.getElementById("main-notification-container");
    const notification = document.createElement("div");
    notification.className = "notification " + modifier;
    notification.innerHTML = html;
    target.prepend(notification);
    console.log("test");
}


function main() {
    create_notification("WARNING: This page is currently under construction and is not done yet!", "m-warning")
    create_3d_canvas();
}



document.addEventListener("DOMContentLoaded", function(){
   main();
});
