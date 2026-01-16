import {Component} from '@angular/core';

@Component({
  selector: 'app-root',
  template: `
    Hello {{ city }}!,
  `,
  standalone: true,
})
export class AppComponent {
  city = 'Philipoppolis';
}
