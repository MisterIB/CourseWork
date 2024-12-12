CREATE TABLE forwards (
    firstName VARCHAR(50),
    lastName VARCHAR(50),
    age INT, 
    team VARCHAR(20), 
    goals INT, 
    assists INT, 
    pim INT
);

CREATE TABLE teams (
    team VARCHAR(50),
    foundation INT,
    coach VARCHAR(50)
);

CREATE TABLE defenders (
    firstName VARCHAR(50),
    lastName VARCHAR(50), 
    age INT, 
    team VARCHAR(20), 
    goals INT, 
    assists INT, 
    pim INT
);

CREATE TABLE goalkeepers (
    firstName VARCHAR(50),
    lastName VARCHAR(50), 
    age INT, 
    team VARCHAR(20), 
    SV INT, 
    assists INT, 
    pim INT
);

CREATE TABLE coaches (
    firstName VARCHAR(50),
    lastName VARCHAR(50), 
    age INT, 
    team VARCHAR(20)
);

CREATE TABLE users (
    username VARCHAR(50),
    right_user VARCHAR(10),
    hash_pswrd VARCHAR(100)
);

/*forwards*/
INSERT INTO forwards (fullname, age, team, goals, assists, pim) VALUES 
('Ivan   Petrov', 19, 'Sibir', 1, 2, 3),
('Alexei Ivanov', 19, 'Sibir', 1, 2, 3),
('Dmitry Pavlov', 19, 'Sibir', 1, 2, 3),
('Sergey Morozov', 19, 'Sibir', 1, 2, 3),
('Mikhail Ivanov', 19, 'Sibir', 1, 2, 3),
('Pavel Kuznetsov', 19, 'Sibir', 1, 2, 3),
('Dmitry  Orlov', 19, 'Voltage', 1, 2, 3),
('Boris Sidorov', 19, 'Voltage', 1, 2, 3),
('Anton Protasov', 19, 'Voltage', 1, 2, 3),
('Maxim Fedorov', 19, 'Voltage', 1, 2, 3),
('Artem Vasilyev', 19, 'Voltage', 1, 2, 3),
('Jack Montgomery', 19, 'Voltage', 1, 2, 3),
('Scott    Dean', 19, 'Steel Bears', 1, 2, 3),
('Jose    Stone', 19, 'Steel Bears', 1, 2, 3),
('Randy  Thomas', 19, 'Steel Bears', 1, 2, 3),
('Charles Duncan', 19, 'Steel Bears', 1, 2, 3),
('Isaac Bennett', 19, 'Steel Bears', 1, 2, 3),
('Roy    Powell', 19, 'Steel Bears', 1, 2, 3);

/*teams*/
INSERT INTO teams (team_name, foundation, coach) VALUES 
('Sibir', 1999, 'Alexandr Alexandrov'),
('Steel Bears', 1967, 'Stepan Stepanov'),
('Voltage', 1983, 'Vladislav Smirnov');

/*defenders*/
INSERT INTO defenders (fullname, age, team, goals, assists, pim) VALUES
('Matthew Hughes', 19, 'Sibir', 1, 2, 3),
('James Garcia', 19, 'Sibir', 1, 2, 3),
('Jerry Taylor', 19, 'Sibir', 1, 2, 3),
('Andrew Robinson', 19, 'Sibir', 1, 2, 3),
('Richard Wise', 19, 'Sibir', 1, 2, 3),
('William Scott', 19, 'Sibir', 1, 2, 3),
('Robert Hall', 19, 'Voltage', 1, 2, 3),
('Tom Flowers', 19, 'Voltage', 1, 2, 3),
('Larry Sanders', 19, 'Voltage', 1, 2, 3),
('William Brooks', 19, 'Voltage', 1, 2, 3),
('David Jackson', 19, 'Voltage', 1, 2, 3),
('Andrew Woods', 19, 'Voltage', 1, 2, 3),
('Larry Hernandez', 19, 'Steel Bears', 1, 2, 3),
('James Mendez', 19, 'Steel Bears', 1, 2, 3),
('Frederick Rodriguez', 19, 'Steel Bears', 1, 2, 3),
('Leroy Ray', 19, 'Steel Bears', 1, 2, 3),
('Douglas Lewis', 19, 'Steel Bears', 1, 2, 3),
('Charles Wallace', 19, 'Steel Bears', 1, 2, 3);


/*goalkeepers*/
INSERT INTO goalkeepers (fullname, age, team, SV, assists, pim) VALUES
('Artem Ryazanov', 19, 'Sibir', 1, 2, 3),
('Steven Gonzalez', 19, 'Sibir', 1, 2, 3),
('Joseph Thompson', 19, 'Voltage', 1, 2, 3),
('James Rivera', 19, 'Voltage', 1, 2, 3),
('Timothy Watson', 19, 'Steel Bears', 1, 2, 3),
('Dean Thompson', 19, 'Steel Bears', 1, 2, 3);


/*coaches*/
INSERT INTO coaches (fullname, age, team) VALUES
('Alexandr Alexandrov', 52, 'Sibir'),
('Stepan Stepanov', 48, 'Steel Bears'),
('Vladislav Smirnov', 36, 'Voltage');

/*users*/
INSERT INTO users (username, right_user, hash_pswrd) VALUES
('admin', 'admin', '-8846692340522361938'),
('firstUser', 'regular', '-5430802346647957293');
