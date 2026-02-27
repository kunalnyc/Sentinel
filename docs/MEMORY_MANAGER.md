# Memory Manager

## WHY memory management exists
Memory management exists to control how RAM is used.

## What a page is
Memory in modern systems is divided into fixed-size blocks called pages.

In 32-bit x86 systems, a page is usually:
4 KB (4096 bytes)

## The Bitmap system
Sentinel OS uses a bitmap-based page allocator.

A bitmap is a simple array where:
Each bit represents one memory page.

0 = free

1 = used

Example:

Bitmap:  1 1 0 0 1 0 0 0
          ↑ ↑ ↑ ↑ ↑ ↑ ↑ ↑
          P0 P1 P2 P3 P4 P5 P6 P7

This means:

Page 0 → Used

Page 1 → Used

Page 2 → Free

Page 3 → Free

Page 4 → Used

Page 5 → Free

Page 6 → Free

Page 7 → Free

This system is:

Simple

Fast

Memory efficient
Deterministic
Perfect for early-stage kernel design.

## allocate_page() — how it works
The allocate_page() function:

Scans the bitmap for the first free bit (0).

Marks that bit as used (1).

Calculates the physical address of that page.

Returns the page address.

Conceptual flow:

Scan bitmap →
Find free bit →
Mark it used →
Return physical address

for each bit in bitmap:
    if bit == 0:
        set bit = 1
        return page_address

## free_page() — how it works
The free_page() function:

Takes a page address.
Converts it to a page index.
Clears the corresponding bitmap bit.
Marks the page as free.

Page address →
Compute page index →
Set bitmap[index] = 0
## How it protects the Trust Registry
The Trust Registry is a critical security structure.

If it gets modified:
Malicious processes could register themselves.
Security enforcement collapses.

So Sentinel must:
Reserve specific pages for the Trust Registry.
Mark them as kernel-only.
Never expose those pages to user processes.
Never allow free_page() to free them.

Implementation idea:
During boot:
Allocate fixed pages for registry.
Mark them permanently used in bitmap.
Do not allow reallocation.
Future enhancement:
Map Trust Registry as read-only.
Use paging to enforce supervisor-only access

Mark as non-executable.

That aligns with Sentinel’s identity-first architecture.