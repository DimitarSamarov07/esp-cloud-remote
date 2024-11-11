//popup
const openButton = document.getElementById("openPopup");
const closeButton = document.getElementById("closePopup");
const popup = document.getElementById("popup");

openButton.addEventListener("click", () => {
    popup.classList.add("open");
});

closeButton.addEventListener("click", () => {
    popup.classList.remove("open");
});

//toggle ESP
const toggle = document.getElementById("toggleLed");
const params = new URLSearchParams();

let isToggled = false;

toggle.addEventListener("change", () => {
    isToggled = isToggled ? false : true;

    let state = isToggled ? 'ON' : 'OFF';
    params.set('led_state', state);
    
    const queryString = params.toString();
    const url = `http://93.155.224.232:8690/led?${queryString}`;
    fetch(url, {
        method: "POST"
    }); 
});