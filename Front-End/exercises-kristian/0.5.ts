import {Component} from '@angular/core';

@Component({
    selector: 'app-root',
    template: `@for (os of operatingSystems; track os.id) {
      {{ os.name }}
    }
  `,
})
export class AppComponent {
    operatingSystems = [{id: 'win', name: 'Windows'}, {id: 'osx', name: 'MacOS'}, {id: 'linux', name: 'Linux'}];}

{
    [{id: 0, name: 'Sarah'},
        {id: 1, name: 'Amy'},
        {id: 2, name: 'Rachel'},
        {id: 3, name: 'Jessica'},
        {id: 4, name: 'Poornima'}];
}
