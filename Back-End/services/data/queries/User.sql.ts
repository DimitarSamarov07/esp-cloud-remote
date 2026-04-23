export class UserQueries {
    static readonly ADD_USER = `INSERT INTO Users (Username, Email, Password, IsAdmin)
                                VALUES (?, ?, ?, 0)`
}