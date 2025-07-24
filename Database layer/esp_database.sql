CREATE DATABASE IF NOT EXISTS esp_remote;
use esp_remote;

CREATE TABLE Users (
                       ID INT AUTO_INCREMENT PRIMARY KEY,
                       Username VARCHAR(255) NOT NULL UNIQUE,
                       Password VARCHAR(255) NOT NULL
);


CREATE TABLE ACL_Table (
                           ID INT AUTO_INCREMENT PRIMARY KEY,
                           UserID INT NOT NULL,
                           Topic VARCHAR(255) NOT NULL,
                           RW INT NOT NULL, -- 1 = read, 2 = write, 3 = read+write
                           FOREIGN KEY (UserID) REFERENCES Users(ID)
);

CREATE TABLE Superusers (
                            ID INT AUTO_INCREMENT PRIMARY KEY,
                            Username VARCHAR(255) NOT NULL UNIQUE
);





