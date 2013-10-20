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
 *   The node object of the webform for which an email was confirmed
 *
 * @param $submission
 *   The submission object of the webform submission where the user
 *   just confirmed his/her email address
 */
function hook_email_confirmed($node, $submission) {
  db_query(
    'INSERT INTO {my_confirmed_submission_list} ' .
    '  VALUES (:nid, :sid) ',
    array(':nid' => $node->nid, ':sid' => $submission->sid)
  );
}

/**
 * React on an email confirmation request that has expired according
 * to the maximum request lifetime that the admin had set
 *
 * @param $node
 *   The node object of the webform for which an email confirmation
 *   request just expired
 *
 * @param $submission
 *   The submission object of the webform submission for which the
 *   email confirmation request just exirted
 */
function hook_confirmation_request_expired($node, $submission) {
  // delete the webform submission that was not confirmed
  webform_submission_delete($node, $submission);
}
