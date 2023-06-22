import { Console } from "./console.js"
import { Clined } from "./protocol.js"
import { copyright, tips } from "./strings.js"

let service;
let inactiveTim = null;

const highlighter = document.createElement("link");
highlighter.media = "screen,print";
highlighter.type  = "text/css";
highlighter.rel   = "stylesheet";

const banner = document.getElementById("banner");
const bannerState = document.getElementById("banner-state");

const consoleRoot = document.getElementById("history");
const cliDiv = document.getElementById("cmdcont");
const cliTxt = document.getElementById("cmdline");
const cliPre = document.getElementById("cmdprev");
const cliBtn = document.getElementById("cmdpost");

const con = new Console(consoleRoot, cliTxt, cliDiv);

const connectingPrompt = consoleRoot.firstElementChild;

const switchTheme = (htmlAttr, cssDir) => {
    highlighter.href = cssDir;
    document.documentElement.setAttribute("data-theme", htmlAttr);
}

if (window.matchMedia && window.matchMedia('(prefers-color-scheme: dark)').matches) {
    switchTheme("dark", "prism-dark.css");
} else {
    switchTheme("", "prism-light.css");
}

document.getElementsByTagName('head')[0].appendChild(highlighter);

/* close code */
const FAILCONN = 0;
const ACCIDENT = 1;
const MANRESET = 2;
const INACTIVE = 3;
const LONGCONN = 4;
const FATALERR = 5;

let closeCode = FAILCONN;
let responded = false;

const wssProtocol = location.protocol == "https:" ? "wss:" : "ws:";
const serviceAddr = `${wssProtocol}//${location.host}/ws`;

const recoverSend = () => {
    const lastContent = con.pending().firstElementChild;
    lastContent.innerHTML = `
        ${cliPre.innerHTML}${lastContent.innerHTML}
    `.trim();
    cliPre.innerHTML = "";
    con.finalize();
    // con.recoverInput();
    responded = true;
};

const handler = {
    onacceped: () => {
        recoverSend();
    },

    onbanned: () => {

    },

    oncontinue: indent => {
        // con.recoverInput();
        responded = true;
        cliPre.innerHTML += con.withdraw() + '\n';
        while(indent-- > 0) con.insertTab();
    },

    ondata: (msg, src) => {
        recoverSend();
        const trimmed = msg.trim();
        if(trimmed.length != 0) 
            con.receive(Prism.highlight(trimmed, Prism.languages.cpp, 'cpp'), src);
    },

    onrecoverable: (reason, detail) => {
        recoverSend();
        con.error(`${reason}\n${detail}`);
    },

    onfatal: e => {
        closeCode = FATALERR;
        con.error(e);
    },

    onhello: () => {
        connectingPrompt.remove();
        cliDiv.style.display = "block";
        cliPre.innerHTML = "";
        con.recoverInput();
        bannerConnect();
        responded = true;
        if(cliTxt.innerHTML == "") {
            const hint = document.createElement("code");
            hint.classList.add("smpcode");
            cliTxt.append(hint);
        }
    },

    oninfo: msg => {
        recoverSend();
        con.info(msg.trim());
    },

    /* * * * * * * * * * * * * * * * *
     *           *  joined * timeout *
     * * * * * * * * * * * * * * * * *
     * responded *  normal *  ? ? ?  *
     * * * * * * * * * * * * * * * * *
     *  awaiting *  killed * timeout *
     * * * * * * * * * * * * * * * * */
    onjoined: () => {
        if(responded) return; // Normal join
        con.finalize();
        con.error("Execution aborted. Possible security policy violation"); 
        con.recoverInput();
    },

    onkilled: () => {
        con.finalize();
        con.recoverInput();
    },

    onlongconn: () => {
        closeCode = LONGCONN;
    },

    onrejected: msg => {
        con.finalize();
        con.error(msg);
        con.recoverInput();
    },

    onwarning: msg => {
        con.warning(msg);
    },

    onopen: () => {
        closeCode = ACCIDENT;
    },
    
    onclose: () => {
        switch(closeCode) {
            case FAILCONN:
                break;

            case ACCIDENT:
                con.error(`Server connection lost`);
                break;
            
            case MANRESET:
                con.warning("Disconnected from the server");
                // service = new Clined(serviceAddr, handler);
                break;

            case INACTIVE:
                con.warning("Server disconnected due to inactivity");
                break;

            case LONGCONN:
                con.warning("Session reached maximum duration");
                break;

            case FATALERR:
                break;
        }
        cliDiv.style.display = "none";
        closeCode = FAILCONN;
        bannerDisconn();
    },

    onconnecterror: e => {
        connectingPrompt.remove();
        con.error(`Service unavailable`);
    },

    ondisconnected: e => {
        cliDiv.style.display = "none";
        con.error(`Server connection lost`);
    }
};

service = new Clined(serviceAddr, handler);

const bannerDisconn = () => {
    banner.onclick = () => {
        if(service.getState() == WebSocket.CLOSED) {
            service = new Clined(serviceAddr, handler);
            consoleRoot.append(connectingPrompt);
        }
    };
    bannerState.innerHTML = "Stopped";
    bannerState.style.color = "orangered";
};

const bannerConnect = () => {
    banner.onclick = () => {
        if(service.getState() == WebSocket.OPEN) {
            service.close();
            closeCode = MANRESET;
        }
    };
    bannerState.innerHTML = "Running";
    bannerState.style.color = "#6BF";
};

con.onsubmit = msg => {
    if(msg.trim().length != 0){
        if(responded) {
            con.pend(cliTxt.innerHTML, msg);
            service.interpret(msg);
        } else {
            con.send(cliTxt.innerHTML, msg);
            service.emulinput(msg + '\n');
        }

        responded = false;
        if(!!inactiveTim) clearTimeout(inactiveTim);
        inactiveTim = setTimeout(() => {
            if(service.getState() == WebSocket.OPEN) {
                service.close();
                closeCode = INACTIVE;
            }
        }, 480000);
    }
};

con.onchange = (msg, prevCaretPos) => {
    if(responded) {
        cliTxt.innerHTML = Prism.highlight(msg, Prism.languages.cpp, 'cpp');
        con.setCaretPosition(prevCaretPos);
    }
};

document.getElementById("hdr-btn-reset").onclick = () => service.softreset();

document.getElementById("hdr-btn-about").onclick = () => {
    con.info("");
    con.editRawHTML(con.lastElem(), copyright);
};

const hints = document.getElementById("banner-hints");

hints.onmouseenter = () => {
    const top = tips.shift();
    hints.innerHTML = top;
    tips.push(top);
};

hints.onmouseleave = () => {
    hints.innerHTML = "Tips";
};

document.getElementById("banner-mask").onclick = () => {
    const top = tips.shift();
    con.info("");
    con.editRawHTML(con.lastElem(), top);
    tips.push(top);
};

setInterval(() => service.keepalive(), 60000);

document.body.setAttribute('spellcheck', false);
