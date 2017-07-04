import { Component, OnInit } from '@angular/core';
import { Title } from '@angular/platform-browser';
import {Router, ActivatedRoute} from '@angular/router';
import { ModelService } from '../shared/shared';


@Component({
  selector: 'editFactors',
  providers: [Title],
  templateUrl: 'app/bayes/edit-factors.component.html'
})  
export class EditFactorsComponent implements OnInit{

  factorId : string;
  constructor(private serv : ModelService,  private rs: ActivatedRoute) {     
  }

  ngOnInit() {
    //this.rs.params
    //  .map(params => params.id)
    //  .flatMap(id => id)
    //  .subscribe(id => this.factorId = id);
    this.factorId = this.rs.snapshot.params['id'];
  }



}

