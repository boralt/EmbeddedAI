import { NgModule }      from '@angular/core';
import { BrowserModule } from '@angular/platform-browser';
import { HttpModule } from '@angular/http';
import { FormsModule }   from '@angular/forms';
import {Response} from '@angular/http';
import { AppComponent }  from './app.component';
import { RouterModule , Routes}   from '@angular/router';
//import { BAYES_DIRECTIVES} from './bayes/bayes'
import {PageNotFoundComponent } from './not-found.component';
// import {CfgEditComponent, EditFactorsComponent, ViewFactorsComponent} from './bayes/bayes';
import {CfgEditComponent} from './bayes/cfg-edit.component';
import {EditFactorsComponent} from './bayes/edit-factors.component';
import {ViewFactorsComponent} from './bayes/view-factors.component';
import {ModelService } from './shared/shared';


const appRoutes: Routes = [
  
  { path: 'cfgEditComponent', component: CfgEditComponent },
  { path: 'viewFactorsComponent', component: ViewFactorsComponent},
  { path: 'editFactorsComponent',  component: EditFactorsComponent },
  //{ path: 'editFactorsComponent/:id',  component: EditFactorsComponent },
 
  { path: '',
    redirectTo: '/cfgEditComponent',
    pathMatch: 'full'
  },
  { path: '**', component: PageNotFoundComponent } 
];

const nonRoutes : Routes = [];

@NgModule({
  imports:      [
      RouterModule.forRoot(appRoutes), 
      BrowserModule, HttpModule, FormsModule
       
      
  ],
  providers : [ModelService],
  declarations: [ AppComponent, CfgEditComponent, ViewFactorsComponent, EditFactorsComponent, PageNotFoundComponent ],
  bootstrap:    [ AppComponent ]
})
export class AppModule { }
