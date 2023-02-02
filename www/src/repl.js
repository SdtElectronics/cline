import { Console } from "./console.js"
import { Clined, INTERPRET, SOFTRESET, DISCARDML } from "./protocol.js"
import { copyright, tips } from "./strings.js"

let service;

const banner = document.getElementById("banner");
const bannerState = document.getElementById("banner-state");

const consoleRoot = document.getElementById("history");
const cliDiv = document.getElementById("cmdcont");
const cliTxt = document.getElementById("cmdline");
const cliPre = document.getElementById("cmdprev");
const cliBtn = document.getElementById("cmdpost");

const con = new Console(consoleRoot, cliTxt, cliDiv);

const connectingPrompt = consoleRoot.firstElementChild;

/* close code */
const FAILCONN = 0;
const ACCIDENT = 1;
const MANRESET = 2;
const INACTIVE = 3;

let closeCode = FAILCONN;
let responded = false;

const serviceAddr = "ws://192.168.137.60:7770";

const recoverSend = () => {
    const lastContent = con.lastSent().firstElementChild;
    lastContent.innerHTML = `
        ${cliPre.innerHTML}${lastContent.innerHTML}
    `.trim();
    cliPre.innerHTML = "";
    con.resolve(con.lastSent());
    con.recoverInput();
    responded = true;
};

const handler = {
    onacceped: () => {
        recoverSend();
    },

    onbanned: () => {

    },

    oncontinue: indent => {
        con.recoverInput();
        responded = true;
        cliPre.innerHTML += con.removeLastSent() + '\n';
        while(indent-- > 0) con.insertTab();
    },

    ondata: msg => {
        recoverSend();
        const trimmed = msg.trim();
        if(trimmed.length != 0) 
            con.receive(Prism.highlight(trimmed, Prism.languages.cpp, 'cpp'));
    },

    onrecoverable: (reason, detail) => {
        recoverSend();
        con.error(`${reason}\n${detail}`);
    },

    onfatal: e => {
        con.error(e);
    },

    onhello: () => {
        connectingPrompt.remove();
        cliDiv.style.display = "block";
        cliPre.innerHTML = "";
        cliTxt.innerHTML = "";
        con.recoverInput();
        bannerConnect();
        responded = true;
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
        con.reject(con.lastSent());
        con.error("Execution aborted. Possible security policy violation"); 
        con.recoverInput();
    },

    onkilled: () => {
        con.reject(con.lastSent());
        con.recoverInput();
    },

    onwarning: msg => {
        con.warning(msg);
    },

    onsleep: () => {
        closeCode = INACTIVE;
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
        con.send(cliTxt.innerHTML, msg);
        service.send(`${INTERPRET}${msg}`);
        con.disableInput();
        responded = false;
    }
};

con.onchange = (msg, prevCaretPos) => {
    cliTxt.innerHTML = Prism.highlight(msg, Prism.languages.cpp, 'cpp');
    con.setCaretPosition(prevCaretPos);
};

document.getElementById("hdr-btn-reset").onclick = () => {
    if(service.getState() == WebSocket.OPEN) {
        service.send(`${SOFTRESET}`);
    }
};

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

document.body.setAttribute('spellcheck', false);
