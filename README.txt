How to use
==========
*Depends on Webform 6.x-3.x. It won't work with the 6.x-2.x or older. Please skip to the end of this document if you are upgrading from a previous version of Webform_confirm_email.*

Webform_confirm_email is a simple addon module for Webform. By default, Webform allows a form creator to define email messages to be sent under some circumstances. Webform_confirm_email lets you specify if certain messages should only be sent after another email address has been confirmed.

Example: Letter writing campaign
================================
Given a webform with these 5 components:

* From email
* From name
* To email
* Hard-coded email text
* Optional email text

The hard-coded email text would be the main body of the letter. The optional email text would suggest more text but would be editable by the user.

From email and name are the user's. To email is the destination address where the letter is sent. The letter will appear to be sent from the user's name and email, but only after the user confirm it is in fact his email address.

When a user submits a letter, he receives an email message containing a link he must follow to confirm he can at least read messages for the email he gave.

When the user follows the confirmation link, he is asked (again) to confirm he is indeed completing the letter campaign process.

Only when he clicks yes to confirm his email address will the letter actually be sent to destination, using the user's name and email as the source.

The user can also click no to report abuse but that feature isn't complete yet. I'm open to suggestions.


Configuration
=============
You will only notice it is installed when visiting a Webform Emails tab. This is where Webform lets you specify messages to send. Webform_confirm_email adds 3 radio buttons for each defined message. Without it or by default, messages are always sent. Webform_confirm_email provides 2 new choices:

* **Confirmation**
* **Conditional**

Check **Confirmation** if you want this message to be sent in all cases. Make sure its template includes the *%confirm_url* token. This turns into the URL a user needs to click to confirm he holds a given email address.

Check **Conditional** if you want a message to be sent only after an email address is confirmed. If you make a message **Conditional** make sure there is at least another message configured as a **Confirmation**, otherwise your **Conditional** message will never get sent.

Any **Confirmation** will send all **Conditional** messages.

Email message contents (templates) are configured as usual, through the Webform interface.

Updating / Installing
=====================
It doesn't handle updating from earlier versions. You must disable and uninstall webform_confirm_email completely (Drupal should remove its database tables) before proceeding. It's a good idea to completely remove the module folder after you uninstalled it.

Untar the module and upload it to your site like any other module. Enable it. It should create a few database tables.

Todo
====
* Confirmation A or B for Conditional B
* Confirmation C and B for Conditional B
* Other configurations
* Presets
* Report abuse feature
* ...

