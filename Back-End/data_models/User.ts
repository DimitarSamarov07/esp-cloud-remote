class User {
    Email: string;
    Username: string;
    Password: string;

    constructor(email: string, username: string, password: string) {
        this.Email = email;
        this.Username = username;
        this.Password = password;
    }

}

export default User;