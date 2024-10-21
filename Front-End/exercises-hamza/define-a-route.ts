// app.routes.ts
import {Routes} from '@angular/router';
import {HomeComponent} from './home/home.component';
import {UserComponent} from './user/user.component';

export const routes: Routes = [{
  path: '',
  title: 'App Home Page',
  component: HomeComponent,
},];

// home.component.ts
import {Component} from '@angular/core';

@Component({
  selector: 'app-home',
  template: `
    <div>Home Page</div>
  `,
  standalone: true,
})
export class HomeComponent {}

