import { Console } from "./console.js"
import { Clined, INTERPRET, SOFTRESET, DISCARDML } from "./protocol.js"

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
const NOACTIVE = 3;

let closeCode = FAILCONN;

// const serviceAddr = "ws://localhost:7770";
const serviceAddr = "ws://192.168.137.60:7770";

const recoverSend = () => {
    const lastContent = con.lastSent().firstElementChild;
    lastContent.innerHTML = `
        ${cliPre.innerHTML}\n${lastContent.innerHTML}
    `.trim();
    cliPre.innerHTML = "";
    con.resolve(con.lastSent());
    con.recoverInput();
};

const handler = {
    onacceped: () => {
        recoverSend();
    },

    onbanned: () => {

    },

    oncontinue: indent => {
        con.recoverInput();
        cliPre.innerHTML += con.removeLastSent() + '\n';
        cliTxt.innerHTML = " ".repeat(indent * 4);
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
        closeCode = ACCIDENT;
        connectingPrompt.remove();
        cliDiv.style.display = "block";
        con.recoverInput();
        bannerConnect();
    },

    oninfo: msg => {
        recoverSend();
        con.info(msg.trim());
    },

    onjoined: () => {
        con.reject(con.lastSent());
        con.recoverInput();
    },

    onwarning: msg => {
        con.warning(msg);
    },

    onsleep: () => {
        closeCode = NOACTIVE;
    },

    onopen: () => {},
    
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

            case NOACTIVE:
                con.warning("Server disconnected due to inactivity");
                break;
        }
        cliDiv.style.display = "none";
        closeCode = FAILCONN;
        bannerDisconn();
    },

    onconnecterror: e => {
        connectingPrompt.remove();
        con.error(`Service unavailable: ${e}`);
    },

    ondisconnected: e => {
        cliDiv.style.display = "none";
        con.error(`Server connection lost: ${e}`);
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

document.body.setAttribute('spellcheck', false);
