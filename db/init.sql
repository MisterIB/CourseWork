CREATE TABLE forwards (
    name VARCHAR(50), 
    age INT, 
    team VARCHAR(20), 
    goals INT, 
    assists INT, 
    pim INT, 
    awards INT
);

CREATE TABLE users (
    userID INT,
    userName VARCHAR(50),
    right VARCHAR(30)
);

CREATE TABLE hashes (
    userID INT,
    hash VARCHAR(50)
);
