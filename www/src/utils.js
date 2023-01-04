export class ElemTemplate {
    constructor(name, init) {
        this.template = document.getElementById(name);
        this.init = init;
    }

    instantiate(...arg){
        const res =  this.template.content.firstElementChild.cloneNode(true);
        if(this.init){
            this.init(res, ...arg);
        }
        return res;
    }
}
