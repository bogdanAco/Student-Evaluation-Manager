DROP TABLE IF EXISTS files;
CREATE TABLE files (
	file_id INT NOT NULL,
	table_name VARCHAR(512) NOT NULL,
	file_name VARCHAR(512) NOT NULL,
	owner INT NOT NULL,
	row_count INT NOT NULL,
	folder INT,
    CONSTRAINT files_pk PRIMARY KEY (file_id)
);
INSERT INTO files VALUES (0, 'table0', 'Tabela test', 0, 600, 0);
DROP TABLE IF EXISTS backup;
CREATE TABLE backup (
	backup_name VARCHAR(512) NOT NULL,
	expire_date DATE NOT NULL,
	owner INT NOT NULL
);
DROP TABLE IF EXISTS folders;
CREATE TABLE folders (
	folder_id INT NOT NULL,
	folder_name VARCHAR(100) NOT NULL,
	folder_parent VARCHAR(100) NOT NULL,
	CONSTRAINT folders_pk PRIMARY KEY (folder_id)
);
INSERT INTO folders VALUES (0, 'Root', '');
DROP TABLE IF EXISTS current_ids;
CREATE TABLE current_ids (
	type VARCHAR(10) NOT NULL,
	val INT NOT NULL
);
INSERT INTO current_ids VALUES ('user', 1);
INSERT INTO current_ids VALUES ('file', 1);
INSERT INTO current_ids VALUES ('folder', 1);
DROP TABLE IF EXISTS users;
CREATE TABLE users (
	user_id INT NOT NULL,
	user_name VARCHAR(512) NOT NULL,
	passwd VARCHAR(512) NOT NULL,
	access_level INT NOT NULL,
	CONSTRAINT users_pk PRIMARY KEY (user_id)
);
INSERT INTO users VALUES (0, '7f8895b9bc1d4ce5a646bff247d2f46c', 
						  '7f8895b9bc1d4ce5a646bff247d2f46c', 1);
DROP TABLE IF EXISTS table0;
CREATE TABLE table0 (
	row_index INT NOT NULL,
	row_timestamp VARCHAR(25) NOT NULL,
	field0 VARCHAR(512),
	field1 VARCHAR(512),
	field2 VARCHAR(512),
	field3 VARCHAR(512),
	field4 VARCHAR(512),
	field5 VARCHAR(512),
    CONSTRAINT row_index_pk PRIMARY KEY (row_index)
);
