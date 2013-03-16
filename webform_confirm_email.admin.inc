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
    'SELECT eid from {webform_emails} WHERE nid = %d AND email = %d',
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
  $form_state = array();
  $form = drupal_retrieve_form('webform_email_edit_form', $form_state, $node, $email);
  drupal_prepare_form('webform_email_edit_form', $form, $form_state);
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

  $form['#submit'][] = '_webform_confirm_email_edit_confirmation_request_email_submit';

  return drupal_render_form('webform_email_edit_form', $form);
}

/**
 * submithandler for confirmation request emails edit form
 */
function _webform_confirm_email_edit_confirmation_request_email_submit($form, &$form_state) {

  if (isset($form['eid']['#value']) == TRUE) {
    $obj['eid']        = $form['eid']['#value'];
    $obj['nid']        = $form['#node']->nid;
    $obj['email_type'] = (int) 1;

    drupal_write_record(
      'webform_confirm_email',
      $obj,
      array('nid', 'eid', 'email_type')
    );
  }
}


/**
 * Implements hook_form_FORM_ID_alter()
 *
 * this alters the webform_emails_form form; it is split in 3 sections, 1 section
 * for standard emails, one for confirmation request emails and one for confirmation
 * emails
 */
function webform_confirm_email_form_webform_emails_form_alter(&$form, &$form_state) {

  $form['emails']['add'] = $form['add'];
  unset($form['add']);
  foreach(array('option', 'custom', 'component') as $add_option) {
    $form['emails']['add']['emails_' . $add_option] = $form['emails']['add']['email_' . $add_option];
    unset($form['emails']['add']['email_' . $add_option]);
  }
  $form['emails']['add_button'] = $form['add_button'];
  unset($form['add_button']);
  $form['emails']['add_button']['#value']  = t('Add standard email');
  $form['emails']['add_button']['#submit'] = array('webform_emails_form_submit');

  $query = db_query(
    'SELECT eid, email_type '.
      'FROM {webform_confirm_email} ' .
        'WHERE nid = %d ' .
        'AND   email_type IN (1, 2)',
    (int) $form['#node']->nid
  );

  $result = array();
  while ($row = db_fetch_array($query)) {
    $result[$row['eid']] = $row['email_type'];
  }

  foreach(array(1 => 'confirmation_request', 2 => 'confirmation') as $email_type_id => $email_type_name) {

    $form[$email_type_name] = array();

    foreach($result as $eid => $email_type) {

      if ($email_type == $email_type_id) {
        $form[$email_type_name][$eid]['email']   = $form['emails'][$eid]['email'];
        $form[$email_type_name][$eid]['subject'] = $form['emails'][$eid]['subject'];
        $form[$email_type_name][$eid]['from']    = $form['emails'][$eid]['from'];

        unset($form['emails'][$eid]);
      }
    }

    $form[$email_type_name]['add'] = array(
      '#theme' => 'webform_email_add_form',
      '#tree'  => FALSE,
    );

    $form[$email_type_name]['add'][$email_type_name . '_option'] = array(
      '#type' => 'radios',
      '#options' => array(
        'custom'    => t('Address'),
        'component' => t('Component value'),
      ),
      '#default_value' => 'custom',
    );

    $form[$email_type_name]['add'][$email_type_name . '_custom'] = array(
      '#type'      => 'textfield',
      '#size'      => 24,
      '#maxlength' => 500,
    );

    $form[$email_type_name]['add'][$email_type_name . '_component'] = array(
      '#type'    => 'select',
      '#options' => webform_component_list($form['#node'], 'email_address', FALSE),
      );

    if (empty($form[$email_type_name]['add'][$email_type_name . '_component']['#options']) == TRUE) {
      $form[$email_type_name]['add'][$email_type_name . '_component']['#options'][''] = t('No available components');
      $form[$email_type_name]['add'][$email_type_name . '_component']['#disabled'] = TRUE;
      }

    if ($email_type_name == 'confirmation_request') {
      $button_label = t('Add confirmation request mail');
      $submit_callback = array('webform_emails_form_submit');
    }
    elseif ($email_type_name == 'confirmation') {
      $button_label = t('Add confirmation mail');
      $submit_callback = array('webform_confirm_email_confirmation_email_add');
    }

    $form[$email_type_name]['add_button'] = array(
      '#type'   => 'submit',
      '#value'  => $button_label,
      '#submit' => $submit_callback,
      '#weight' => 45,
    );
  }
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
    if (count($eids) > 2) {
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


/**
 * Implements hook_form_FORM_ID_alter()
 *
 * This hook adds a radio button list titled "MESSAGE TYPE" to the webform
 * "Edit e-mail settings" site
 */
function webform_confirm_email_form_webform_email_edit_form_alter(&$form, &$form_state) {
  $form['#submit'][] = '_webform_confirm_email_edit_std_email_submit';
}

/**
 * This implements the submit handler callback for an webform
 * email settings edit event
 *
 * @param $form
 *   the form for which webform email settings are changed
 *
 * @param $form_state
 *   the form_state of the form
 */
function _webform_confirm_email_edit_std_email_submit($form, &$form_state) {

  if (isset($form['eid']['#value']) == TRUE) {
    $obj['eid'] = $form['eid']['#value'];
    $obj['nid'] = $form['#node']->nid;
    $obj['email_type'] = (int) 0;

    drupal_write_record(
      'webform_confirm_email',
      $obj,
      array('nid', 'eid', 'email_type')
    );
  }
}

