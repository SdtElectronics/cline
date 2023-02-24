/* request code */
const INTERPRET = 0;
const SOFTRESET = 1;
const DISCARDML = 2;
const KEEPALIVE = 9;

export class Clined {
    constructor(address, handler) {
        this.connected = false;
        this.handler = handler;
        this.server = new WebSocket(address);
        this.server.onopen = this.onopen;
        this.server.onmessage = this.onmessage;
        this.server.onclose = this.onclose;
        this.server.onerror = this.onerror;
    }

    send = msg => {
        this.server.send(msg);
    }

    close = () => {
        this.server.close();
    }

    onopen = () => {
        this.handler.onopen();
    }

    onclose = () => {
        this.connected = false;
        this.handler.onclose();
    }

    onmessage = event => {
        const evBody = event.data.slice(1);
        const header = event.data[0];
        switch(header) {
            case 'a':
                this.handler.onacceped();
                break;

            case 'b':
                this.handler.onbanned();
                break;

            case 'c':
                this.handler.oncontinue(parseInt(evBody));
                break;
            
            case 'd':
                this.handler.ondata(evBody, "Value of last expression");
                break;
 
            case 'e':
                this.handler.onrecoverable(
                    "Uncaught exception:", evBody);
                break;

            case 'f':
                this.handler.onfatal(evBody);
                break;
            
            case 'g':
                this.handler.onrecoverable(
                    "Compilation error:", evBody);
                break;

            case 'h':
                this.connected = true;
                this.handler.onhello();
                break;

            case 'i':
                this.handler.oninfo(evBody);
                break;

            case 'j':
                this.handler.onjoined();
                break;

            case 'k':
                this.handler.onkilled();
                break;

            case 'l':
                this.handler.onlongconn();
                break;

            case 'm':
                
                break;

            case 'n':
                this.handler.onrecoverable(
                    "Interpreter internal exception:", evBody);
                break;

            case 'o':
                break;

            case 'p':
                
                break;

            case 'q':
                
                break;

            case 'r':
                this.handler.onrejected(evBody);
                break;

            case 's':
                break;

            case 't':
                this.handler.onrecoverable(
                    "Soft reset: blocked too long", "");
                break;

            case 'u':
                this.handler.onrecoverable(
                    "Uncaught unknown exception", "");
                break;
                
            case 'v':
                
                break;

            case 'w':
                this.handler.onwarning(evBody);
                break;

            case 'x':
                this.handler.ondata(evBody, "stdout");
                break;

            case 'y':
                this.handler.ondata(evBody, "stderr");
                break;

            case 'z':
                
                break;

            default:
                console.log(`Unknown response: ${evBody}`);
        }
    }

    onerror = event => {
        if(this.connected) {
            this.connected = false;
            this.handler.ondisconnected(event);
        } else {
            this.handler.onconnecterror(event);
        }
    }

    interpret(msg) {
        this.send(`${INTERPRET}${msg}`);
    }

    softreset() {
        if(this.getState() == WebSocket.OPEN) {
            this.send(`${SOFTRESET}`);
        }
    }

    keepalive() {
        if(this.getState() == WebSocket.OPEN) {
            this.send(`${KEEPALIVE}`);
        }
    }

    getState() {
        return this.server.readyState;
    }
}
