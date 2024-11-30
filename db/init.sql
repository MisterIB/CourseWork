CREATE TABLE forwards (
    fullname VARCHAR(50), 
    age INT, 
    team VARCHAR(20), 
    goals INT, 
    assists INT, 
    pim INT, 
    awards INT
);

CREATE TABLE teams (
    team_name VARCHAR(50),
    foundation_date INT,
    defenders INT,
    forwards INT,
    goalkeepers INT,
    awards INT
);

CREATE TABLE defenders (
    fullname VARCHAR(50), 
    age INT, 
    team VARCHAR(20), 
    goals INT, 
    assists INT, 
    pim INT, 
    awards INT
);

CREATE TABLE goalkeepers (
    fullname VARCHAR(50), 
    age INT, 
    team VARCHAR(20), 
    SV INT, 
    assists INT, 
    pim INT, 
    awards INT
);

CREATE TABLE coaches (
    fullname VARCHAR(50), 
    age INT, 
    team VARCHAR(20),
    awards INT
);

/*forwards*/
INSERT INTO forwards (fullname, age, team, goals, assists, pim, awards) VALUES ('Igor Battalov', 19, 'Sibir', 1, 2, 3, 4);

INSERT INTO forwards (fullname, age, team, goals, assists, pim, awards) VALUES ('Vlad Teterin', 19, 'Sibir', 1, 2, 3, 4);

INSERT INTO forwards (fullname, age, team, goals, assists, pim, awards) VALUES ('Anton Protasov', 19, 'Sibir', 1, 2, 3, 4);

INSERT INTO forwards (fullname, age, team, goals, assists, pim, awards) VALUES ('Egor Cherednyak', 19, 'Sibir', 1, 2, 3, 4);

INSERT INTO forwards (fullname, age, team, goals, assists, pim, awards) VALUES ('Ilya Avramchuk', 19, 'Sibir', 1, 2, 3, 4);

INSERT INTO forwards (fullname, age, team, goals, assists, pim, awards) VALUES ('Denis Kolodin', 19, 'Sibir', 1, 2, 3, 4);

/*teams*/
INSERT INTO teams (team_name, foundation_date, defenders, forwards, goalkeepers, awards) VALUES ('Sibir', 1999, 3, 0, 0, 100);

/*defenders*/
INSERT INTO defenders (fullname, age, team, goals, assists, pim, awards) VALUES ('Ivan Merzov', 19, 'Sibir', 1, 2, 3, 4);

/*goalkeepers*/
INSERT INTO goalkeepers (fullname, age, team, SV, assists, pim, awards) VALUES ('Artem Ryazanov', 19, 'Sibir', 1, 2, 3, 4);
