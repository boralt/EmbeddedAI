import { Injectable }     from '@angular/core';
import { Http, Response, Headers, RequestOptions, URLSearchParams } from '@angular/http';
import { Observable }     from 'rxjs/Rx';
import 'rxjs/Rx';


@Injectable()
export default class AiService {
  private aiUrl = 'http://localhost:3000/cgi-bin/airt.cgi.exe';

  constructor (private http: Http) {

  }

  toJson(r : Response) : string {
     return r.text();
  }

  getAiRt(req : string) : Observable<string> {      
     let headers = new Headers();
      headers.append('Content-Type', 'application/x-www-form-urlencoded');
      const body = new URLSearchParams();
      body.set('req', req);

      var myHttpRequest: Observable<Response> = this.http.post(this.aiUrl, body.toString(), headers);
      return myHttpRequest.map<Response, string>(this.toJson);
  }
}
