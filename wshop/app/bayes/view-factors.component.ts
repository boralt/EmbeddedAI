import { Component } from '@angular/core';
import {ModelService } from '../shared/shared';
import { Router}   from '@angular/router';
import { Title } from '@angular/platform-browser';




@Component({
  selector: 'viewFactors',
  providers: [Title],
  templateUrl: 'app/bayes/view-factors.component.html'
})  
export class ViewFactorsComponent {
  
  arListVars: string[];
  arListFactors: string[];
  selectedVar: string;


  constructor(private serv: ModelService, private router : Router) { 
    this.arListVars = serv.getVars();
    this.arListFactors = serv.getFactors();
  }

  onAddFactor() {
    this.serv.addFactor(this.selectedVar);
    this.arListFactors = this.serv.getFactors();
    
  }

  onEditFactor(v : string) {
    this.router.navigate(["editFactorsComponent/" , {id:v}]);
    //this.router.navigate(["editFactorsComponent/"  + v ]);
    
  } 

  onChangeVar(e : any) {
    this.selectedVar = e; 
    
  }


}



