import { Component } from '@angular/core';
import { Title } from '@angular/platform-browser';
import {ModelService } from '../shared/shared';


@Component({
  selector: 'manageVars',
  providers: [Title],
  templateUrl: 'app/bayes/manage-vars.component.html'
})  
export class ManageVarsComponent {

  vars : string[];
  constructor(private serv: ModelService) { }

  onAddVar(v:string) {
    this.serv.addVar(v);
    this.vars = this.serv.getVars();
  }

  onDelVar(v:string) {
     this.serv.deleteVar(v);
     this.vars = this.serv.getVars();
  }

}

