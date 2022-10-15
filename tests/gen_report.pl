#!/usr/bin/perl
use v5.10;
use utf8;
use autodie;
use Excel::Writer::XLSX;

my $workbook = Excel::Writer::XLSX->new( 'report.xlsx' );
my $worksheet = $workbook->add_worksheet();
$workbook->add_format(xf_index => 0, font => 'Courier New', align => 'right');

my $row = 0;
my $col = 0;
sub writesheet {
    my $key = shift;
    my $value = shift;
    $col == 1 && $worksheet->write($row, 0, $key);
    $worksheet->write($row, $col, $value);
    $row++;
}

chomp(@lines = <>);

my $num = '\s*(\d+(?:\.\d+)?)';
foreach (@lines) {
    if (/BLEVEL_EXPAND_BUF_KEY:\s*(\d+)/) {
        if ($col > 0) {
            writesheet 'expand count', $expand_count;
            writesheet 'expand blevel count', $expand_blevel_count;
            writesheet 'expand last size', $expand_last_size;
            writesheet 'expand clevel size', $expand_clevel_size;
            writesheet 'expand clevel count', $expand_clevel_count;
            writesheet 'expand pairs per clevel', $expand_pairs_per_clevel;
        }
        $col++;
        $row = 0;
        $expand_count = 0;
        writesheet "expand buf key", $1;
    } elsif (/EXPANSION_FACTOR:\s*(\d+)/) {
        writesheet "expand factor", $1;
    } elsif (/start to expand combotree/) {
        $expand_count++;
    } elsif (/load:${num}${num}/) {
        writesheet "load iops", $2;
    } elsif (/put:${num}${num}/) {
        writesheet 'put iops', $2;
    } elsif (/get:${num}${num}/) {
        writesheet 'get iops', $2;
    } elsif (/scan 10:${num}${num}/) {
        writesheet 'scan 10 iops', $2;
    } elsif (/scan 100:${num}${num}/) {
        writesheet 'scan 100 iops', $2;
    } elsif (/scan 1000:${num}${num}/) {
        writesheet 'scan 1000 iops', $2;
    } elsif (/scan 10000:${num}${num}/) {
        writesheet 'scan 10000 iops', $2;
    } elsif (/clevel time:${num}/) {
        writesheet 'clevel time', $1;
    } elsif (/entries:${num}/) {
        writesheet 'entries', $1;
    } elsif (/clevels:${num}/) {
        writesheet 'clevels', $1;
    } elsif (/clevel percent:${num}/) {
        writesheet 'clevel_percent', $1 . '%';
    } elsif (/bytes-per-pair:${num}/) {
        writesheet 'bytes per pair', $1;
    } elsif (/suffix 1 count:${num}/) {
        writesheet 'suffix 1 count', $1;
    } elsif (/finish expanding combotree. current size is ${num}, current entry count is ${num}/) {
        $expand_last_size = $1;
        $expand_last_count = $2;
    } elsif (/data in clevel: ${num}, clevel count: ${num}, pairs per clevel: ${num}/) {
        $expand_clevel_size = $1;
        $expand_clevel_count = $2;
        $expand_blevel_count = $expand_last_count;
        $expand_pairs_per_clevel = $3;
    } elsif (/usage:\s*(\S*)/) {
        writesheet '容量', $1;
    }
}
writesheet 'expand count', $expand_count;
writesheet 'expand blevel count', $expand_blevel_count;
writesheet 'expand last size', $expand_last_size;
writesheet 'expand clevel size', $expand_clevel_size;
writesheet 'expand clevel count', $expand_clevel_count;
writesheet 'expand pairs per clevel', $expand_pairs_per_clevel;

$workbook->close();