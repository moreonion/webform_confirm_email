=======
Read me
=======

:Author: Robin Millette <robin@millette.info>
:Date: 2010/03/27
:$Id$

Summary
=======
`Webform confirm email address`_ is used in combination with Webform_ as a `letter campaign`_ engine. Install and configure this module if you need to verify the email addresses your users provide before sending out letters. It prevents people from pretending they are someone else.

TODO
====
* Povide better feedback to the user.
* Use a template for the confirmation email.
* Cleanup unconfirmed webforms.

Requirements
============
I wrote it for Drupal 6. It depends on the Webform_ 6.x-2.x module.

Installation
============
Install as usual, see http://drupal.org/node/70151 for further information.

Configuration
=============
First, set up up a Webform as explained in `letter campaign`_ or using your own method. Next, find the email component of that form you want to use as the sender's email. In **Advanced settings**, check **Must confirm this email address**. You are done.

Customization
=============
Nothing I can think of for now. Maybe I'll implement a template for the confirmation email if there is demand.

Troubleshooting
===============
See the `support issues`_ on the Drupal website.

FAQ
===
How does it work?
  Quite frankly, I'm not really sure :) But seriously, the module intercepts the letter before it is sent with a *hook_mail_alter* and sends its own confirmation mail instead to the person listed as the source. That email contains a cryptographically signed link the user must follow to confirm his identity. Once the website gets that, the letter we intercepted gets sent.

Why isn't this in Webform?
  Dunno. I never asked the Webform_ maintainers if they were interested. Maybe it'll make its way there and this more will become obselete.

What was this tested with?
  Drupal 6.16 and Webform 6.x-2.9. Let me know if you try it with other versions.

Contact
=======
Current maintainer
------------------
Robin Millette (millette) - http://drupal.org/user/49105

Sponsor
-------
This module provided to you by la Ligue des Contribuables (du Québec) and Muniduweb.

Colophon
========
This document is written using the ReStructuredText markup. Go get Docutils_ if you'd like to see this document in a format such as HTML, LaTeX or PDF. You may also convert this file using my `Text filter web service`.

.. _Webform: http://drupal.org/project/webform
.. _`Webform confirm email address`: http://drupal.org/project/webform_confirm_email
.. _Docutils: http://docutils.sourceforge.net/
.. _`Text filter web service`: http://textfilters.appspot.com/
.. _`letter campaign`: http://dharmatech.org/blog/creating_letter_editor_form_drupal/06/11/09
.. _`support issues`: http://drupal.org/project/issues/webform_confirm_email?categories=support


