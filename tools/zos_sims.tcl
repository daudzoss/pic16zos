#!/usr/bin/tclsh

set command "exec yes | ./zos_rol"

 foreach {old new} {
     5 1  1 1  1 2  2 3  3 4  4 5
     5 2  2 2  2 4  4 1  1 3  3 5
     5 3  3 3  3 1  1 4  4 2  2 5
     5 4  4 4  4 3  3 2  2 1  1 5
     5 5
 } {
     lappend command | ./zos_rol $old $new 800
 }

puts $command
catch {eval [split $command]} output
puts $output