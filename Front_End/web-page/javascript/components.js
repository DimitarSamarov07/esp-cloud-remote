class EasyHeader extends HTMLElement {
    connectedCallback() {
        this.innerHTML = '<header>\n' +
            '    <article class="logo">\n' +
            '        <a href="#">Admin Panel</a>\n' +
            '    </article>\n' +
            '    <ul class="nav-links">\n' +
            '        <li><a href="index.html">Home</a></li>\n' +
            '        <li><a href="#">About Us</a></li>\n' +
            '        <li><a href="connect-esp-intro.html">Connect new ESP</a></li>\n' +
            '    </ul>\n' +
            '</header>' +
            '';
    }
}

class EasyFooter extends HTMLElement {
    connectedCallback(){
        this.innerHTML = '<footer>\n' +
            '    <article class="footer">\n' +
            '        <section class="contact">\n' +
            '            <div class="logo">\n' +
            '                <img src="../images/cloud-icon.png" alt="cloud-icon">\n' +
            '                <h3>ESP Cloud Remote</h3>\n' +
            '            </div>\n' +
            '            <ul>\n' +
            '                <li><p class="list-head">Need help?</p></li>\n' +
            '                <li><a href="#">Contact us via email at support@example.com</a></li>\n' +
            '            </ul>\n' +
            '        </section>\n' +
            '        <section class="links">\n' +
            '            <ul>\n' +
            '                <li><p class="list-head">Company</p></li>\n' +
            '                <li><a href="#">Terms & Conditions</a></li>\n' +
            '                <li><a href="#">Privacy Policy</a></li>\n' +
            '            </ul>\n' +
            '            <ul>\n' +
            '                <li><p class="list-head">Download</p></li>\n' +
            '                <li><a href="#">Microsoft Store</a></li>\n' +
            '                <li><a href="#">Google Play</a></li>\n' +
            '                <li><a href="#">Apple Store</a></li>\n' +
            '            </ul>\n' +
            '            <ul>\n' +
            '                <li><p class="list-head"><a href="https://github.com/DimitarSamarov07/esp-cloud-remote" target="_blank">View\n' +
            '                    us on GitHub</a></p></li>\n' +
            '            </ul>\n' +
            '        </section>\n' +
            '    </article>\n' +
            '    <article class="copyright">\n' +
            '        <hr>\n' +
            '        <p>ESP Cloud Remote @ 2024</p>\n' +
            '    </article>\n' +
            '</footer>';
    }
}

customElements.define('easy-header', EasyHeader);
customElements.define('easy-footer', EasyFooter);