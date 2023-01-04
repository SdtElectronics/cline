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
                this.handler.ondata(evBody);
                break;
 
            case 'e':
                this.handler.onrecoverable(
                    "Uncaught user exception:", evBody);
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
                this.handler.onrecoverable(
                    "Execution aborted. Possible security policy violation", "");
                break;

            case 'l':
                this.handler.onwarning("Too much data written to stdout. Discarding...");
                break;

            case 'm':
                
                break;

            case 'n':
                this.handler.onrecoverable(
                    "Interpreter internal exception:", evBody);
                break;

            case 'o':
                this.handler.onrecoverable(
                    "Invalid input: too long", "");
                break;

            case 'p':
                
                break;

            case 'q':
                
                break;

            case 'r':
                
                break;

            case 's':
                this.handler.onsleep();
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
                this.handler.ondata(evBody);
                break;

            case 'y':
                
                break;

            case 'z':
                
                break;

            default:
                this.handler.ondata(evBody);
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

    getState() {
        return this.server.readyState;
    }
}

/* request code */
export const INTERPRET = 0;
export const SOFTRESET = 1;
export const DISCARDML = 2;
