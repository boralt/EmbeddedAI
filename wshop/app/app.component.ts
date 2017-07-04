import { Component } from '@angular/core';
import {AiService } from './shared/shared';

import { RouterModule, Routes }   from '@angular/router';

import {Response} from '@angular/http';

import {CfgEditComponent} from './bayes/cfg-edit.component';
import {EditFactorsComponent} from './bayes/edit-factors.component';
import {ViewFactorsComponent} from './bayes/view-factors.component';


/*
//import { 
//  ROUTER_PROVIDERS,
//  RouteConfig,
//  ROUTER_DIRECTIVES 
// } from '@angular/router';
*/



@Component({
  selector: 'my-app',
//  directives: [ROUTER_DIRECTIVES],
//  providers: [SHARED_PROVIDERS, HTTP_PROVIDERS, ROUTER_PROVIDERS],

  templateUrl: 'app/app.component.html' ,
  providers: [AiService]
})
export class AppComponent  { 
  name = 'Angular'; 
  inpJson : string;
  jsonRes : string;
  finished : boolean;

  constructor(private aiServ : AiService) {
    this.jsonRes = "N/A";
    this.inpJson = "Start here";
    this.finished = false;
  }

  onSend() {
    this.finished = false;
    this.aiServ.getAiRt(this.inpJson).subscribe(
      (value : string) => { this.jsonRes = value },
      (error : string)  => this.jsonRes = "Err",
      () => this.finished = true);

  } 

}
