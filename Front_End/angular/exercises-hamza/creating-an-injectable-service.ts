// car.service.ts
import {Injectable} from '@angular/core';

@Injectable({
  providedIn: 'root'
})
export class CarService {
  cars = ['Sunflower GT', 'Flexus Sport', 'Sprout Mach One'];

  getCars(): string[] {
    return this.cars;
  }

  getCar(id: number) {
    return this.cars[id];
  }
}

//app.component.ts
import {Component, inject} from '@angular/core';
import {CarService} from './car.service';

@Component({
  selector: 'app-root',
  template: '<p> {{ carService.getCars() }} </p>',
  standalone: true,
})


export class AppComponent {
  carService = inject(CarService);
}

