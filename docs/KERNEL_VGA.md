# VGA - Video Graphics Array
# VGA Text Mode

## What is VGA Text Mode?
VGA Video graphics array where it's decode the text

## Memory Address 0xB8000
0xB8000 ~ This is → where the SCREEN is mapped in memory

## How Characters Work
The Chars working procces require (2 Bytes)
0xB8000 ~ This is → where the SCREEN is mapped in memory
0x0F ~ This is the Color where whenever text will load so how will the interface will be visually
it Both require 2 Byte Each like 

0x0F = 0000 1111
         ↑      ↑
       black   white
       background  text

## Color Byte 0x0F
Color Byte require for Visulas graphics (0x0F) <-- This also stored in memory address of 0xB8000

## Where Kernel is Loaded
0x100000  → where your KERNEL is loaded in memory