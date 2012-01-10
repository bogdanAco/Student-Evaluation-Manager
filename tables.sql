DROP TABLE IF EXISTS files;
CREATE TABLE files (
	file_id INT NOT NULL,
	table_name VARCHAR(128) NOT NULL,
	file_name VARCHAR(128) NOT NULL,
	owner INT NOT NULL,
	row_count INT NOT NULL,
	folder INT NOT NULL,
    CONSTRAINT files_pk PRIMARY KEY (file_id)
);
DROP TABLE IF EXISTS backup;
CREATE TABLE backup (
	backup_name VARCHAR(128) NOT NULL,
	expire_date DATE NOT NULL,
	owner INT NOT NULL
);
DROP TABLE IF EXISTS folders;
CREATE TABLE folders (
	folder_id INT NOT NULL,
	folder_name VARCHAR(128) NOT NULL,
	folder_parent VARCHAR(128) NOT NULL,
	CONSTRAINT folders_pk PRIMARY KEY (folder_id)
);
INSERT INTO folders VALUES (0, 'Root', '');
DROP TABLE IF EXISTS current_ids;
CREATE TABLE current_ids (
	type VARCHAR(10) NOT NULL,
	val INT NOT NULL
);
INSERT INTO current_ids VALUES ('user', 0);
INSERT INTO current_ids VALUES ('file', 0);
INSERT INTO current_ids VALUES ('folder', 1);
DROP TABLE IF EXISTS users;
CREATE TABLE users (
	user_id INT NOT NULL,
	user_name VARCHAR(128) NOT NULL,
	passwd VARCHAR(64) NOT NULL,
	public_key VARCHAR(1024) NOT  NULL,
	CONSTRAINT users_pk PRIMARY KEY (user_id)
);
DROP TABLE IF EXISTS rights;
CREATE TABLE rights (
	table_id INT NOT NULL,
	user_id INT NOT NULL,
	column_id INT NOT NULL
);
DROP TABLE IF EXISTS access_keys;
CREATE TABLE access_keys (
	table_id INT NOT NULL,
	user_id INT NOT NULL,
	access_key VARCHAR(256) NOT NULL,
	signed_key VARCHAR(256) NOT NULL
);
DROP TABLE IF EXISTS tables_settings;
CREATE TABLE tables_settings (
	table_id INT NOT NULL,
	column_id INT NOT NULL,
	width INT NOT NULL,
	header_text VARCHAR(128)
);