import {Moment} from 'moment'


class AirConditioner {
    ID: string;
    Name: string | null;
    Brand: string | null;
    Model: string | null;
    LastSeen: Moment | null;

    constructor(id: string, name: string | null,brand: string | null, model: string | null, lastseen: Moment | null,) {
        this.ID = id;
        this.Name = name;
        this.Brand = brand;
        this.Model = model;
        this.LastSeen = lastseen;
    }

}

export default AirConditioner;