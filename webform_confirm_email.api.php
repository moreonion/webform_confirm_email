<?php
/**
 * @file
 *
 * @author    Matthias Weiss <matthias@more-onion.com>
 * @copyright Copyright (c) 2013 copyright
 */


/**
 * React on an email that was confirmed when the user clicked
 * the confirmation link
 *
 * @param $node
 *   The node containing the webform where a email was confirmed
 *
 * @param $submission
 *   The original submission of the user that just confirmed his/her
 *   email address
 */
function hook_email_confirmed($node, $submission) {
  db_query(
    'INSERT INTO {my_confirmed_submission_list} ' .
    '  VALUES (:nid, :sid) ',
    array(':nid' => $node->nid, ':sid' => $submission->sid);
}