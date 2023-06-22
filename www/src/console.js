import { LineHist } from "./linehist.js"
import { ElemTemplate } from "./utils.js"

export class Console {
    constructor(hist, input, inputWrapper = null, historySize = 1000) {
        this.hist = hist;
        this.inputWrapper = inputWrapper != null ? inputWrapper : input;
        this.input = input;
        this.lastElem_ = hist.firstElementChild;
        this.linehist = new LineHist(historySize);
        this.linehist.push();

        this.lineTemplate = new ElemTemplate(
            "log",
            (elem, type, msg) => {
                elem.classList.add(type);
                elem.querySelector(".line-content").textContent = msg;
            }
        );

        input.onkeydown = ev => {
            if (ev.key === 'Enter') {
                ev.preventDefault();
                if(ev.shiftKey){
                    document.execCommand('insertLineBreak');
                    const msg = input.innerText;
                    const prevCaretPos = this.getCaretPosition();
                    this.onchange(msg, prevCaretPos);
                } else {
                    const msg = input.innerText;
                    this.onsubmit(msg);
                }
            } else if(ev.key === 'Tab') {
                this.insertTab();
                ev.preventDefault();
            } else if (ev.key === 'ArrowDown') {
                /* navigate through history when caret hit boundary */
                if(!this.linehist.onLastLine()) {
                    const sel = window.getSelection();
                    const prevCaretPos = sel.getRangeAt(0).cloneRange();

                    sel.modify("move","forward","lineboundary");
                    const offset = sel.focusOffset;
                    sel.modify("move","forward","character");
                    if (offset == sel.focusOffset) {
                        const msg = this.linehist.move(-1);
                        this.onchange(msg);
                    } else {
                        sel.removeAllRanges();
                        sel.addRange(prevCaretPos);
                    }
                }
                
            } else if(ev.key === 'ArrowUp') {
                if(!this.linehist.onFirstLine()) {
                    const sel = window.getSelection();
                    const prevCaretPos = sel.getRangeAt(0).cloneRange();

                    sel.modify("move","backward","lineboundary");
                    const offset = sel.focusOffset;
                    sel.modify("move","backward","character");
                    if (offset == sel.focusOffset) {
                        ev.preventDefault();
                        ev.stopPropagation();                       // Avoid scrolling up
                        if(this.linehist.onLastLine()) this.linehist.edit(input.innerText);
                        const msg = this.linehist.move(1);
                        this.onchange(msg);
                    } else {
                        sel.removeAllRanges();
                        sel.addRange(prevCaretPos);
                    }
                }
            } else if(ev.key != "Control" && ev.key != "Shift") {
                setTimeout(() => {
                    const msg = input.innerText;
                    const prevCaretPos = this.getCaretPosition();
                    this.onchange(msg, prevCaretPos);
                }, 0);
            }
        };
    }

    lastElem() {
        return this.lastElem_;
    }

    pending() {
        return this.pending_;
    }

    pend(msg, hist, title) {
        this.send(msg, hist, title, "line-pending");
        this.pending_ = this.lastElem_;
    }

    finalize() {
        this.pending_.classList.remove("line-pending");
        this.pending_.classList.add("line-sent");
    }

    withdraw() {
        const ret = this.pending_.firstElementChild.innerHTML;
        this.pending_.remove();
        return ret;
    }

    send(msg, hist, title, type = "line-sent") {
        this.lastElem_ = this.lineTemplate.instantiate(type, "");
        this.lastElem_.title = title;
        this.editRawHTML(this.lastElem_, msg);
        this.hist.append(this.lastElem_);
        this.linehist.edit(hist);
        this.linehist.push();
        this.input.innerHTML = "";
    }

    receive(msg, title) {
        this.lastElem_ = this.lineTemplate.instantiate("line-received", "");
        this.lastElem_.title = title;
        this.editRawHTML(this.lastElem_, msg);
        this.hist.append(this.lastElem_);
    }

    info(msg) {
        this.lastElem_ = this.lineTemplate.instantiate("line-info", msg);
        this.hist.append(this.lastElem_);
    }

    warning(msg) {
        this.lastElem_ = this.lineTemplate.instantiate("line-warning", msg);
        this.hist.append(this.lastElem_);
    }

    error(msg) {
        this.lastElem_ = this.lineTemplate.instantiate("line-error", msg);
        this.hist.append(this.lastElem_);
    }

    recoverInput() {
        this.input.contentEditable = "true";
        this.inputWrapper.style.filter = "";
        this.input.focus();
    }

    disableInput() {
        this.input.contentEditable = "false";
        this.inputWrapper.style.filter = "grayscale(1)";
    }

    enabledInput() {
        return this.input.contentEditable == "plaintext-only";
    }

    editRawHTML(elem, msg) {
        elem.querySelector(".line-content").innerHTML = msg;
    }

    insertTab() {
        this.input.focus();
        document.execCommand('insertHTML', false, '&#009');
    }

    getCaretPosition_(parent, node, offset, stat) {
        if (stat.done) return stat;
      
        let currentNode = null;
        if (parent.childNodes.length == 0) {
            stat.pos += parent.textContent.length;
        } else {
            for (let i = 0; i < parent.childNodes.length && !stat.done; i++) {
                    currentNode = parent.childNodes[i];
                if (currentNode === node) {
                    stat.pos += offset;
                    stat.done = true;
                    return stat;
                } else this.getCaretPosition_(currentNode, node, offset, stat);
            }
        }
        return stat;
    }

    getCaretPosition() {
        const sel = window.getSelection();
        return this.getCaretPosition_(
            this.input, sel.focusNode, sel.focusOffset, { pos: 0, done: false }
        );
    }
      
    setCaretPosition_(parent, range, stat) {
        if (stat.done) return range;
        
        if (parent.childNodes.length == 0) {
            if (parent.textContent.length >= stat.pos) {
                range.setStart(parent, stat.pos);
                stat.done = true;
            } else {
                stat.pos = stat.pos - parent.textContent.length;
            }
        } else {
            for (let i = 0; i < parent.childNodes.length && !stat.done; i++) {
                this.setCaretPosition_(parent.childNodes[i], range, stat);
            }
        }
        return range;
    }

    setCaretPosition(pos) {
        const sel = window.getSelection();
        sel.removeAllRanges();

        if(pos == undefined) {
            setTimeout(() => {
                const range = document.createRange();//Create a range (a range is a like the selection but invisible)
                range.selectNodeContents(this.input);//Select the entire contents of the element with the range
                range.collapse(false);

                sel.addRange(range);
            }, 0);
            return;
        }

        const range = this.setCaretPosition_(this.input, document.createRange(), {
            pos: pos.pos,
            done: false,
        });
        range.collapse(true);
        sel.addRange(range);
    }
}
