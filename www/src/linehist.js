class CircularBuffer {
    constructor(bufferLength) {
        this.buffer = [];
        this.pointer = -1;
        this.bufferLength = bufferLength;
    }
    
    push(element) {
        if(this.buffer.length === this.bufferLength) {
            this.buffer[this.pointer] = element;
        } else {
            this.buffer.push(element);
        }
        this.pointer = (this.pointer + 1) % this.bufferLength;
    }
    
    get(i) {
        return this.buffer[i];
    }
    
    getSize() {
        return this.buffer.length;
    }

    getLast(i) {
        let idx = this.pointer - i;
        return this.buffer[idx < 0 ? idx + this.bufferLength : idx];
    }

    setLast(i, val) {
        let idx = this.pointer - i;
        this.buffer[idx < 0 ? idx + this.bufferLength : idx] = val;
    }
    
    getLower() {
        return this.buffer.slice(this.pointer + 1);
    }

    getUpper() {
        return this.buffer.slice(0, this.pointer + 1);
    }
}

export class LineHist {
    constructor(historySize){
        this.history = new CircularBuffer(historySize);
    }

    edit(msg) {
        this.history.setLast(0, msg);
    }

    push() {
        this.cur = 0;
        this.history.push("");
    }

    move(step) {
        const tmp = this.cur + step;
        if(tmp < this.history.getSize() && tmp >= 0){
            this.cur = tmp;
        }
        return this.history.getLast(this.cur);
    }

    export() {
        return this.getLower().join("\n") + "\n" +
               this.getUpper().join("\n");
    }

    onLastLine() {
        return this.cur == 0;
    }

    onFirstLine() {
        return this.cur == this.history.getSize() - 1;
    }

    cur = 0;
}
