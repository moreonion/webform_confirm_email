<?php


/**
 * Implements hook_webform_component_delete().
 *
 * If a webform component is being deleted we check
 * if this component is an email address field and if so
 * we delete the confirmation emails
 *
 * @param $component
 *   The Webform component being deleted.
 */
function webform_confirm_email_webform_component_delete($component) {
  $nid = (int) $component['nid'];
  $cid = (int) $component['cid'];
  if (!$nid || !$cid) {
    return;
  }

  $results = db_query(
    'SELECT eid ' .
      'FROM {webform_emails} ' .
        'WHERE nid = %d ' .
        'AND email = %d ',
    $nid,
    $cid
  );
  while ($wfemail = db_fetch_object($results)) {
    if (empty($wfemail->eid)) {
      continue;
    }
    $eid = (int) $wfemail->eid;
    db_query(
      'DELETE from {webform_confirm_email} WHERE nid = %d AND eid = %d',
      $nid,
      $eid
    );
    // FIXME
    // We need to handle deletions from
    // webform_confirm_email_confirm and webform_confirm_email_code.
    // webform module should take care of the following but since it doesn't...
    db_query(
      'DELETE from {webform_emails} WHERE nid = %d AND eid = %d',
      $nid,
      $eid
    );
  }
}

/**
 * Implements hook_form_FORM_ID_alter().
 *
 * This adds a submit handler callback for the webform settings, so when
 * webform email settings are changed (an webform email is deleted) we
 * can react on this
 */
function webform_confirm_email_form_webform_email_delete_form_alter(&$form, &$form_state) {
  $form['#submit'][] = '_webform_confirm_email_email_delete';
}

/**
 * This implements the submit handler callback for an webform
 * email delete event
 *
 * @param $form
 *   the form from which a webform email is deleted
 *
 * @param $form_state
 *   the form_state of the form
 */
function _webform_confirm_email_email_delete($form, &$form_state) {
  $nid = (int) $form['email']['#value']['nid'];
  $eid = (int) $form['email']['#value']['eid'];
  if (   empty($nid) == FALSE
      && empty($eid) == FALSE) {
    db_query(
      'DELETE FROM {webform_confirm_email} ' .
        'WHERE nid = %d ' .
        'AND   eid = %d',
      $nid,
      $eid
    );
    // FIXME
    // We need to handle deletions from
    // webform_confirm_email_confirm and webform_confirm_email_code.
  }
}

/**
 * callback function set in hook_menu().
 * It creates the email settings form for the confirmation request emails
 */
function webform_confirm_email_confirmation_request_edit($node, $email = array()) {

  include_once drupal_get_path('module', 'webform') . '/includes/webform.emails.inc';

  // copied from form.inc: drupal_get_form
  $form_state = array(
   'storage'   => NULL,
   'submitted' => FALSE,
   'post'      => $_POST,
  );

  $form = drupal_retrieve_form('webform_email_edit_form', $form_state, $node, $email);
  $form['#build_id'] = 'form-' . md5(uniqid(mt_rand(), TRUE));

  drupal_prepare_form('webform_email_edit_form', $form, $form_state);

  unset($form_state['post']);

  $form['#post'] = $_POST;

  $form['#submit'] = array(
    'webform_email_edit_form_submit',
    '_webform_confirm_email_edit_confirmation_request_email_submit',
  );

  drupal_process_form('webform_email_edit_form', $form, $form_state);

  $instructions = $form['template']['tokens']['#value'];
  $search = '<p>' .
    t('You may use special tokens in this field that will be replaced with dynamic values.') .
    '</p>';

  $confirmation_tokens = array(t('%confirm_url - URL needed to confirm email address.'));

  $rep = $search . theme_item_list(
    $confirmation_tokens,
    t('Confirmation variables')
  );
  $form['template']['tokens']['#value'] = str_replace($search, $rep, $instructions);

  return drupal_render_form('webform_email_edit_form', $form);
}

/**
 * submit handler for confirmation request emails edit form
 */
function _webform_confirm_email_edit_confirmation_request_email_submit($form, &$form_state) {

  if (   isset($form_state['values']['eid']) == TRUE
      && isset($form['node']['#value']->nid) == TRUE
      && empty($form['eid']['#value'])       == TRUE /*-> new email*/) {

    $obj['eid']        = $form_state['values']['eid'];
    $obj['nid']        = $form['node']['#value']->nid;

    drupal_write_record(
      'webform_confirm_email',
      $obj
    );
  }
}

/**
 * callback function set in hook_menu().
 * It creates the email settings form for the confirmation request emails
 */
function webform_confirm_email_confirmation_request_delete($node, $email = array()) {

  include_once drupal_get_path('module', 'webform') . '/includes/webform.emails.inc';

  // copied from form.inc: drupal_get_form
  $form_state = array(
   'storage'   => NULL,
   'submitted' => FALSE,
   'post'      => $_POST,
  );

  $form = drupal_retrieve_form('webform_email_delete_form', $form_state, $node, $email);
  $form['#build_id'] = 'form-' . md5(uniqid(mt_rand(), TRUE));

  drupal_prepare_form('webform_email_delete_form', $form, $form_state);

  unset($form_state['post']);

  $form['#post'] = $_POST;

  $form['#submit'] = array(
    'webform_email_delete_form_submit',
    '_webform_confirm_email_confirmation_request_email_delete_submit',
  );

  drupal_process_form('webform_email_delete_form', $form, $form_state);

  return drupal_render_form('webform_email_delete_form', $form);
}

/**
 * submit handler for confirmation request emails edit form
 */
function _webform_confirm_email_confirmation_request_email_delete_submit($form, &$form_state) {

  $node  = $form_state['values']['node'];
  $email = $form_state['values']['email'];

  if (   isset($email['eid']) == TRUE
      && isset($node->nid) == TRUE) {
    db_query(
      'DELETE FROM {webform_confirm_email} ' .
        'WHERE nid = %d ' .
        'AND   eid = %d',
      $node->nid,
      $email['eid']);
  }
}

/**
 * Menu loader callback
 */
function webform_confirmation_email_load($eid, $nid) {

  if ($eid == 'new') {
    $email = array(
      'email'               => '',
      'subject'             => 'default',
      'from_name'           => 'default',
      'from_address'        => 'default',
      'template'            => 'default',
      'excluded_components' => array(),
      'html'                => variable_get('webform_default_format', 0),
      'attachments'         => 0,
    );
  }
  else {
    $query = db_query(
      'SELECT  nid, eid, to_address AS email, from_name, from_address, subject, template, excluded_components, html, attachments '.
        'FROM {webform_confirm_email_confirmation_emails} ' .
          'WHERE nid  = %d ' .
          'AND   eid = %d',
      (int) $nid,
      (int) $eid
    );

    $email = db_fetch_array($query);

    if (variable_get('webform_format_override', 0) != 0) {
      $email['html'] = variable_get('webform_default_format', 0);
    }
  }

  return $email;
}

/**
 * callback function set in hook_menu().
 * It creates the email settings form for the confirmation request emails
 */
function webform_confirm_email_confirmation_edit($node, $email = array()) {

  include_once drupal_get_path('module', 'webform') . '/includes/webform.emails.inc';

  // copied from form.inc: drupal_get_form
  $form_state = array(
   'storage'   => NULL,
   'submitted' => FALSE,
   'post'      => $_POST,
  );

  $form = drupal_retrieve_form('webform_email_edit_form', $form_state, $node, $email);
  $form['#build_id'] = 'form-' . md5(uniqid(mt_rand(), TRUE));

  drupal_prepare_form('webform_email_edit_form', $form, $form_state);

  unset($form_state['post']);

  $form['#post'] = $_POST;

  $form['#submit'] = array(
    '_webform_confirm_email_edit_confirmation_email_submit',
  );

  drupal_process_form('webform_email_edit_form', $form, $form_state);

  return drupal_render_form('webform_email_edit_form', $form);
}

function _webform_confirm_email_edit_confirmation_email_submit($form, &$form_state) {

  // FIXME: ensure that a confirmation request email was defined before

  // copied from webform_email_edit_form_submit

  $node = $form_state['values']['node'];

  // Merge the e-mail, name, address, and subject options into single values.
  $email = array(
    'eid' => $form_state['values']['eid'],
    'nid' => $node->nid,
  );

  foreach (array('email', 'from_name', 'from_address', 'subject') as $field) {
    $option = $form_state['values'][$field . '_option'];
    if ($option == 'default') {
      $email[$field] = 'default';
    }
    else {
      $email[$field] = $form_state['values'][$field . '_' . $option];
    }
  }

  $email['to_address'] = $email['email'];
  unset($email['email']);

  // Ensure templates are unaffected by differences in line breaks.
  $form_state['values']['template']             = str_replace(array("\r", "\n"), array('', "\n"), $form_state['values']['template']);
  $form_state['values']['templates']['default'] = str_replace(array("\r", "\n"), array('', "\n"), $form_state['values']['templates']['default']);

  // Set the template value.
  if (strcmp(trim($form_state['values']['templates']['default']), trim($form_state['values']['template'])) == 0) {
    $email['template'] = 'default';
  }
  else {
    $email['template'] = $form_state['values']['template'];
  }

  // Save the attachment and HTML options provided by MIME mail.
  $email['html']        = (empty($form_state['values']['html']) == TRUE)        ? 0 : 1;
  $email['attachments'] = (empty($form_state['values']['attachments']) == TRUE) ? 0 : 1;

  // Save the list of included components.
  // We actually maintain an *exclusion* list, so any new components will
  // default to being included in the %email_values token until unchecked.
  $included = array_keys(array_filter((array) $form_state['values']['components']));
  $excluded = array_diff(array_keys($node->webform['components']), $included);
  $email['excluded_components'] = implode(',', $excluded);

  if (empty($form_state['values']['eid']) == TRUE) {
    drupal_set_message(t('Email settings added.'));

    db_lock_table('webform_confirm_email_confirmation_emails');

    if (isset($email['eid']) == FALSE) {
      $email['eid'] = db_result(
        db_query(
          'SELECT MAX(eid) ' .
            'FROM {webform_confirm_email_confirmation_emails} '.
              'WHERE nid = %d',
          $email['nid']
        )
      ) + 1;
    }

    drupal_write_record('webform_confirm_email_confirmation_emails', $email);
    db_unlock_tables();

    $form_state['values']['eid'] = $email['eid'];
  }
  else {
    drupal_set_message(t('Email settings updated.'));

    drupal_write_record('webform_confirm_email_confirmation_emails', $email, array('nid', 'eid'));
  }

  $form_state['redirect'] = array('node/' . $node->nid . '/webform/emails');
}

function webform_confirm_email_confirmation_delete($node, $email = array()) {
}

/**
 * Implements hook_form_FORM_ID_alter()
 *
 * this alters the webform_emails_form form; it is split in 3 sections, 1 section
 * for standard emails, one for confirmation request emails and one for confirmation
 * emails
 */
function webform_confirm_email_form_webform_emails_form_alter(&$form, &$form_state) {

  unset($form['add']);

  $form['emails']['add_button'] = $form['add_button'];
  unset($form['add_button']);
  $form['emails']['add_button']['#value']  = t('Add standard email');
  $form['emails']['add_button']['#submit'] = array('webform_emails_form_submit');

  $query = db_query(
    'SELECT eid '.
      'FROM {webform_confirm_email} ' .
        'WHERE nid = %d ' ,
    (int) $form['#node']->nid
  );

  $form['confirmation_request'] = array();
  while ($eid = db_fetch_array($query)) {
    $eid = array_shift($eid);
    $form['confirmation_request'][$eid]['email']   = $form['emails'][$eid]['email'];
    $form['confirmation_request'][$eid]['subject'] = $form['emails'][$eid]['subject'];
    $form['confirmation_request'][$eid]['from']    = $form['emails'][$eid]['from'];

    unset($form['emails'][$eid]);
  }

  $form['confirmation_request']['add_button'] = array(
    '#type'   => 'submit',
    '#value'  => t('Add confirmation request mail'),
    '#submit' => array('webform_confirm_email_confirmation_request_email_add'),
    '#weight' => 45,
  );

  $form['confirmation'] = array();
  $query = db_query(
    'SELECT eid, to_address AS email, subject, from_name, from_address '.
      'FROM {webform_confirm_email_confirmation_emails} ' .
        'WHERE nid = %d ' ,
    (int) $form['#node']->nid
  );
  while ($email = db_fetch_array($query)) {
    $eid = array_shift($email);
    if (empty($email['from_name']) == FALSE ) {
      $email['from'] = '&quot;' . $email['from_name'] . '&quot; &lt;' . $email['from_address'] . '&gt;';
    }
    else {
      $email['from'] = $email['from_address'];
    }
    $form['confirmation'][$eid]['email']['#value']   = $email['email'];
    $form['confirmation'][$eid]['subject']['#value'] = $email['subject'];
    $form['confirmation'][$eid]['from']['#value']    = $email['from'];
  }

  $form['confirmation']['add_button'] = array(
    '#type'   => 'submit',
    '#value'  =>  t('Add confirmation mail'),
    '#submit' =>  array('webform_confirm_email_confirmation_email_add'),
    '#weight' => 45,
  );

}

/**
 * submit handler callback from the confirmation request email add button
 *
 * set a redirect to the path node/xx/webform/confirmation_request/new where
 * in hook_menu a page callback is defined: webform_confirm_email_confirmation_request_edit
 */
function webform_confirm_email_confirmation_request_email_add($form, &$form_state) {
  $form_state['redirect'] = array(
    'node/' . $form['#node']->nid . '/webform/confirmation_request/new',
  );
}

/**
 * submit handler callback from the confirmation email add button
 *
 * set a redirect to the path node/xx/webform/confirmation/new where
 * in hook_menu a page callback is defined: webform_confirm_email_confirmation_edit
 */
function webform_confirm_email_confirmation_email_add($form, &$form_state) {
  $form_state['redirect'] = array(
    'node/' . $form['#node']->nid . '/webform/confirmation/new',
  );
}

/**
 * implements hook_theme_registry_alter().
 *
 * insert 2 theme hooks for the 'webform_emails_form' and the 'webform_email_add_form'
 */
function webform_confirm_email_theme_registry_alter(&$theme_registry) {

  $theme_registry['webform_emails_form']['function']    = 'theme_webform_confirm_email_emails_form';
  $theme_registry['webform_email_add_form']['function'] = 'theme_webform_confirm_email_email_add_form';
}

/**
 * theme hook defined in hook_theme_registry_alter().
 *
 * includes the original theme hook for the 'webform_emails_form' and adds handling of
 * confirmation request emails and confirmation emails
 */
function theme_webform_confirm_email_emails_form($form) {
  // Add CSS to display submission info. Don't preprocess because this CSS file is used rarely.
  drupal_add_css(drupal_get_path('module', 'webform') . '/css/webform-admin.css', 'theme', 'all', FALSE);
  drupal_add_js(drupal_get_path('module', 'webform') . '/js/webform-admin.js', 'module', 'header', FALSE, TRUE, FALSE);

  $node = $form['#node'];

  $header = array(
    t('E-mail to'),
    t('Subject'),
    t('From'),
    array(
      'data'    => t('Operations'),
      'colspan' => 2
    )
  );

  $output = '';
  foreach(array(
      'emails'               => 'Standard emails (always send)',
      'confirmation_request' => 'Confirmation request emails (always send)',
      'confirmation'         => 'Confirmation emails (only send when the confirmation URL is used)') as $email_type => $title) {

    $rows = array();
    $eids = element_children($form[$email_type]);
    if (count($eids) > 1) {
      foreach ($eids as $eid) {
        if ($eid != 'add' && $eid != 'add_button') {
          // Add each component to a table row.
          $rows[] = array(
            drupal_render($form[$email_type][$eid]['email']),
            drupal_render($form[$email_type][$eid]['subject']),
            drupal_render($form[$email_type][$eid]['from']),
            l(t('Edit'),   'node/' . $node->nid . '/webform/' . $email_type . '/' . $eid),
            l(t('Delete'), 'node/' . $node->nid . '/webform/' . $email_type . '/' . $eid . '/delete'),
          );
        }
      }
    }
    else {
      switch($email_type) {
        case 'emails':
          $no_email_comment = t('Currently not sending standard e-mails, add an e-mail recipient below.');
          break;
        case 'confirmation_request':
          $no_email_comment = t('Currently not sending confirmation request e-mails, add an e-mail recipient below.');
          break;
        case 'confirmation':
          $no_email_comment = t('Currently not sending confirmation e-mails, add an e-mail recipient below.');
          break;
      }
      $rows[] = array(array('data' => $no_email_comment, 'colspan' => 5));
    }

    // Add a row containing form elements for a new item.
    $row_add_email = array(
      array(
        'colspan' => 3,
        'data'    => drupal_render($form[$email_type]['add'])
      ),
      array(
        'colspan' => 2,
        'data'    => drupal_render($form[$email_type]['add_button'])
      ),
    );
    $rows[] = array('data' => $row_add_email, 'class' => 'webform-add-form');

    $output .= '<h2>' . $title . '</h2>';
    $output .= theme('table', $header, $rows, array('id' => 'webform-' . $email_type));
  }

  $output .= drupal_render($form);
  return $output;
}

/**
 * theme hook defined in hook_theme_registry_alter().
 *
 * includes the original theme hook for the 'webform_email_add_form' and adds handling of
 * confirmation request emails and confirmation emails
 */
function theme_webform_confirm_email_email_add_form($form) {

  $email_type = $form['#array_parents'][0];

  // Add a default value to the custom e-mail textfield.
  $form[$email_type . '_custom']['#attributes']['rel']   = t('email@example.com');
  $form[$email_type . '_custom']['#attributes']['class'] = 'webform-set-active webform-default-value';
  $form[$email_type . '_option']['custom']['#title']     = $form[$email_type . '_option']['custom']['#title'] . ': ' . drupal_render($form[$email_type . '_custom']);

  // Render the component value.
  $form[$email_type . '_component']['#attributes']['class'] = 'webform-set-active';
  $form[$email_type . '_option']['component']['#title'] = $form[$email_type . '_option']['component']['#title'] . ': ' . drupal_render($form[$email_type . '_component']);

  // For spacing consistency, every option is wrapped in webform-container-inline.
  foreach (element_children($form[$email_type . '_option']) as $option) {
    $form[$email_type . '_option'][$option]['#prefix'] = '<div class="webform-container-inline">';
    $form[$email_type . '_option'][$option]['#suffix'] = '</div>';
  }

  return drupal_render($form);
}
