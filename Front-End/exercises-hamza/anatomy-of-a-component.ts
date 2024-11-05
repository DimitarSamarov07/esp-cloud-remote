  GNU nano 8.0             anatomy-of-a-component.ts
import {Component} from '@angular/core';

@Component({
  selector: 'app-root',
  template: `
    Hello
  `,
  styles: `
    :host {
      color: blue;
    }
  `,
  standalone: true,
})
export class AppComponent {}
