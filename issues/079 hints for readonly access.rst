SECoP Issue 79: hints for readonly access (closed)
==================================================

Motivation
----------

The 'visibility' property does not address the case where the client should
distinguish between read and write access for a module or parameter for a
certain access level.


Proposal
--------

Possible solutions:

1) Add a 'writelevel' property which indicates a different access level for writing than
   for visibility. The 'writelevel' must always be at least as restrictive as the
   'visibility' level and can be omitted when it is equal.
   This is backwards compatible to the previous specification.

2) Like above, but 'writelevel' is appended to the visibility:
   e.g. visibility="user,advanced" or visibility=["user", "advanced"]
   There is a compatibility issue in the case the SEC node is 'newer' than the ECS.

3) Create a new property 'access' as a string composed of at most 3 letters
   indicating either 'a' for full access or 'r' for readonly access, in the order
   'expert', 'advanced' and 'user'.

4) Create a new property 'accessmode' as a string composed of at exactly 3 letters
   indicating either 'w' for full access, 'r' for readonly access, or '-' for no access,
   in the order 'expert', 'advanced' and 'user'. Looks similar than a unix file access
   mode. The readonly flag would in principle get redundant, and the rules for the
   default behaviour might be confusing, e.g. when the module has accessmode "www" and
   a parameter is readonly.

Compatibility for (3) and (4):
   * when the ECS is newer than the SEC node, it has also to check for 'visibility'
   * when the ECS is older than the SEC node, compatibility is lost, except we specify
     that the old 'visibility' must to be given anyway.

.. table:: possible combinations of access hints

     ======== ========== ======== ============= ============= ======== ============ ==========
      expert   advanced   user     visibility    writelevel    access   accessmode   readonly
     ======== ========== ======== ============= ============= ======== ============ ==========
      rd/wr    rd/wr      rd/wr    "user"                      "aaa"    "www"        false
      rd/wr    rd/wr      rd       "user"        "advanced"    "aar"    "wwr"        false
      rd/wr    rd/wr      no       "advanced"                  "aa"     "ww-"        false
      rd/wr    rd         rd       "user"        "expert"      "arr"    "wrr"        false
      rd/wr    rd         no       "advanced"    "expert"      "ar"     "wr-"        false
      rd/wr    no         no       "expert"                    "a"      "w--"        false
      rd       rd         rd       "user"                      "aaa"    "rrr"        true
      rd       rd         no       "advanced"                  "aa"     "rr-"        true
      rd       no         no       "expert"                    "a"      "r--"        true
     ======== ========== ======== ============= ============= ======== ============ ==========

5) access oriented naming:

* "w": full access (write and read)
* "r": read access only
* "": no access

For readonly parameters, "w" and "r" would be the same, so there is an ambiguity
whether to choose "w" or "r". For that reason it might be more clear to
use "a" for full access.

6) restriction oriented naming:

* "": no restriction (full access: read and possibly write)
* "r": read access only
* "n": no access

As a visibility specification is a restriction, we might use the empty string
for no restriction.

The key "expert" is not needed, as on the expert level always full access should
be possible.

.. table:: options with JSON object for "visibility" property

     ======== ========== ======== ============= ================================ ================================
      expert   advanced   user     old specs     access oriented                  restriction oriented
     ======== ========== ======== ============= ================================ ================================
      rd/wr    rd/wr      rd/wr    "user"        {"advanced": "w", "user": "w"}   {"advanced": "" , "user": "" }
      rd/wr    rd/wr      rd                     {"advanced": "w", "user": "r"}   {"advanced": "" , "user": "r"}
      rd/wr    rd/wr      no       "advanced"    {"advanced": "w", "user": "" }   {"advanced": "" , "user": "n"}
      rd/wr    rd         rd                     {"advanced": "r", "user": "r"}   {"advanced": "r", "user": "r"}
      rd/wr    rd         no                     {"advanced": "r", "user": "" }   {"advanced": "r", "user": "n"}
      rd/wr    no         no       "expert"      {"advanced": "" , "user": "" }   {"advanced": "n", "user": "n"}
     ======== ========== ======== ============= ================================ ================================

New servers must use a JSON object for the "visibility" property, client should be ready
to accept the string and treat is as given in above table.

The property on the module level should be taken as a default for all of its parameters.
Especially, the properties of a parameter might override a readonly access specified
by the properties of a module. However, when the module property describes it as not
visible for a certain access level, its parameters should not be accessible at all.

Discussion
==========

Version (1) has better compatibility than 2..4. Version (4) is more self
explaining way to describe access, because its gives the access mode explicitly
for each level separately.
On the meeting of 2023-01-17 it was decidied that a JSON object should be used
as this is most self describing (version 5 and 6).
Here the compatibility is even better, as a new client easily can just replace the
given string by the corresponding JSON object.

On the meeting of 2023-02-14 we redecided to go for option 4, but keep the name "visibility".
