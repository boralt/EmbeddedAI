import { Component } from '@angular/core';
import { Title } from '@angular/platform-browser';
import {ModelService } from '../shared/shared';


@Component({
  selector: 'cfgEdit',
  providers: [Title],
  templateUrl: 'app/bayes/cfg-edit.component.html'
})  
export class CfgEditComponent {

  vars : string[];
  constructor(private serv: ModelService) { }
  onAddVar(v:string) {
    this.serv.addVar(v);
    this.vars = this.serv.getVars();
  }
}

