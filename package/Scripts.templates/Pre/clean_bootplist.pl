#!/usr/bin/perl

use strict;
use YAML::Syck;

our $target_volume;
our $boot_plist_filepath;

our $yaml_file="@YAML_FILE@";

if ($#ARGV < 0) {
   print stderr "A target volume is needed\n";
} else {
   $target_volume=$ARGV[0];
}

$boot_plist_filepath = "${target_volume}/Extra/org.chameleon.Boot.plist";
if ( -f "$boot_plist_filepath" ) {
    main("$yaml_file");
}

sub _do_cmd {
    my ($cmd, $key, $value) = @_;
    my $out;

    $key   =~ s/([\s])/\\$1/g;  # Escape characters in key
    $value =~ s/([\s"])/\\$1/g; # Escape characters in value (space & ")
    my $plistbuddy_command="$cmd :$key $value";

    open ( OUTPUT, "-|", '/usr/libexec/plistbuddy', "-c", "$plistbuddy_command", "$boot_plist_filepath" );
    my $exit_code = $?;
    chomp($out = <OUTPUT>);
    close OUTPUT;
    return $out;
}

sub get_boot_plist_option {
    my ($option) = @_;
    return _do_cmd( "Print", "$option");
}

sub delete_boot_plist_option {
    my ($option) = @_;
    return _do_cmd( "Delete", "$option");
}

sub delete_boot_plist_text_option {
    my ($option, $values) = @_;
    my $current_value = get_boot_plist_option "$option";
    if ( $current_value ne "") {
        foreach my $recognized_value (@{$values}) {
            if ($recognized_value eq $current_value) {
                return _do_cmd( "Delete", "$option");
            }
        }
    }
    return "";
}

sub delete_boot_plist_list_option {
    my ($option, $values) = @_;
    my $current_value = get_boot_plist_option "$option";
    if ( $current_value ne "") {
        my %count;
        my @new_list;
        foreach my $value (@{$values}) {
            $count{$value} = 1;
        }
        foreach my $value (split(/\s+/,$current_value)) {
            if ( not exists $count{$value} ) {
                push @new_list, $value;
            }
        }
        return _do_cmd( "Set", $option, join(' ',@new_list) );
    }
    return "";
}

sub main() {
    # Remove all options that installer can managed
    my ($yaml_file) = @_;
    my $hash_ref = LoadFile($yaml_file) or die "Can't open yaml file\n";
    foreach my $option ( keys %{$hash_ref} ) {
        my $type = $hash_ref->{$option}->{type};
        if ( $type =~ /^bool$/i ) {
            delete_boot_plist_option($option);
        } elsif ( $type =~ /^text$/i ) {
            delete_boot_plist_text_option( $option,
                                           $hash_ref->{$option}->{values} );
        } elsif ( $type =~ /^list$/i ) {
            delete_boot_plist_list_option( $option,
                                           $hash_ref->{$option}->{values} );
        }
    }
}
