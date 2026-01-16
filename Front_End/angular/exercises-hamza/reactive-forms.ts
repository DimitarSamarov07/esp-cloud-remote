import {Component} from '@angular/core';
import {ReactiveFormsModule, FormGroup, FormControl} from '@angular/forms';


@Component({
  selector: 'app-root',
  template: `
    <form
      [formGroup]="profileForm"
      (ngSubmit)="handleSubmit()">
      <label>
        Name {{profileForm.value.name}}
        <input type="text" formControlName="name" />
      </label>
      <label>
        Email {{profileForm.value.email}}
        <input type="email" formControlName="email" />
      </label>
      <button type="submit">Submit</button>
    </form>
  `,
  standalone: true,
  imports: [ReactiveFormsModule],
})
export class AppComponent {
  profileForm = new FormGroup({
    name: new FormControl(''),
    email: new FormControl(''),
  });

  handleSubmit() {
    alert(
      this.profileForm.value.name + ' | ' + this.profileForm.value.email
    );
  }
}
