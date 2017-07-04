import { Injectable }     from '@angular/core';
import { Http, Response, Headers, RequestOptions, URLSearchParams } from '@angular/http';
import { Observable }     from 'rxjs/Rx';
import 'rxjs/Rx';




class Factor {
   vars : string[];
   vals : { [inst:number]:number };
   headVar : string;

   addHeadVar(v:string) {
      this.headVar = v;
   }

   setVars(v:string[]) {
      this.vars = v;
      this.vals = [];
   }

   getVars() : string[] {
      return this.vars;
   }

   setHeaderVar(v:string) {
      this.headVar = v;
   }

}


interface MapOfFactors {
  m: {[v: string] : Factor}; 
}

interface Sample {
   vals : {[v : string]: boolean}
}


@Injectable()
export default class ModelService {
  private aiUrl = 'http://localhost:3000/cgi-bin/airt.cgi.exe';

  private arVars: string[] = [];
  private arFactors : MapOfFactors = <MapOfFactors>{m:{}};
  private sample : Sample; 
  private resSet : string[];

  constructor (private http: Http) {

  }

  init() {

  }

  addVar(v:string) {

     if(this.arVars.filter(item => item==v).length == 0)      
     {
        this.arVars.push(v);
     }
  }
   
  getVars() : string[] {
     return this.arVars;
  }

  deleteVar(v:string)  {
     let i = this.arVars.indexOf(v);
     if (i >=0)
      this.arVars.splice(i, 1);
  }

  addFactor(v:string) {
     if (this.arFactors.m[v] == undefined)
     {
        this.arFactors.m[v] = new Factor;
        this.arFactors.m[v].addHeadVar(v);
     }

  }

  deleteFactor(v:string) {
     delete this.arFactors[v];
  }

  getFactors() : string[] {
     let res : string[] = [];
     for (var f in this.arFactors.m) {
        res.push(f);
     }
     return res;
  }

  setVarsToFactor(v:string, vs:string[]) {
     let f = this.arFactors[v];
     if ( f != undefined)
     {
        f.setVars(vs);
     }
  } 

  getVarsToFactor(v:string) : string[] {
     let f = this.arFactors[v];
     if ( f != undefined)
     {
        return f.getVars(vs);
     }
     return [];
  }


  addVarToSmple(v:string, sample:boolean){
     this.sample[v] =sample;
  }

  deleteVarFromSample(v:string) {
     delete this.sample[v];
  }

  setResSet(v:string[]) {
     this.resSet = v;
  }

  getResSet() : string[] {
     return this.resSet;
  }


}
