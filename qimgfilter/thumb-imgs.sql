
drop table thumb_imgs;
create table thumb_imgs (
    md5 char(32) primary key,
    url varchar(5000) not null,
    raw_url varchar(5000) not null,
    raw_width integer,
    raw_height integer,
    alt text,
    thumb_base64 text not null,
    thumb_content bytea
);

psql -h l-dev1.s.cn3.qunar.com -Upostgre snapshot -c "copy thumb_imgs (md5, url, raw_url, raw_width, raw_height, alt, thumb_base64) from stdin with null as ''"
