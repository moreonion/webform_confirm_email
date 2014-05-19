<?php
/**
 * @file
 */

include_once 'webform_confirm_email.inc';

/**
 * Helper function to propvide a form for filtering for (un)confirmed submissions
 */
function webform_confirm_email_confirmed_filter_form($form, &$form_state) {
  $options = array(
    WEBFORM_CONFIRM_EMAIL_FILTER_NONE        => t('All submissions'),
    WEBFORM_CONFIRM_EMAIL_FILTER_CONFIRMED   => t('Only confirmed submissions'),
    WEBFORM_CONFIRM_EMAIL_FILTER_UNCONFIRMED => t('Only unconfirmed submissions'),
  );
  $query_parameters = drupal_get_query_parameters();
  $form['confirmed']= array(
    '#type'          => 'radios',
    '#title'         => t('Filter for (un)confirmed submissions'),
    '#default_value' => isset($query_parameters['confirmed']) ? $query_parameters['confirmed'] : WEBFORM_CONFIRM_EMAIL_FILTER_NONE,
    '#options'       => $options,
    '#access'        => TRUE,
  );

  $form['submit'] = array(
    '#type'  => 'submit',
    '#value' => t('Filter submissions'),
  );

  $form['#submit'] = array('webform_confirm_email_confirmed_filter_form_submit');

  return $form;
}

/**
 * Submit handler for filter form for filtering for (un)confirmed submissions
 */
function webform_confirm_email_confirmed_filter_form_submit($form, &$form_state) {
  $path = current_path();
  $query_parameters = drupal_get_query_parameters();
  if (   isset($form_state['values']['confirmed']) == TRUE
      && $form_state['values']['confirmed'] != WEBFORM_CONFIRM_EMAIL_FILTER_NONE) {
    $query_parameters['confirmed'] = $form_state['values']['confirmed'];
  }
  else {
    unset($query_parameters['confirmed']);
  }

  drupal_goto($path, array('absolute' => TRUE, 'query' => $query_parameters));
}

/**
 * Helper function to determine if a "confirmed submissions" filter is set
 */
function _webform_confirm_email_get_submission_filters() {
  $filters = array();

  if (   isset($_GET['confirmed']) == TRUE
      && is_numeric($_GET['confirmed']) == TRUE) {
    switch ($_GET['confirmed']) {
      case WEBFORM_CONFIRM_EMAIL_FILTER_CONFIRMED:
         $filters['confirmed'] = 1;
         break;
      case WEBFORM_CONFIRM_EMAIL_FILTER_UNCONFIRMED:
         $filters['confirmed'] = 0;
         break;
    }
  }
  return $filters;
}

function webform_confirm_email_results_submissions($node, $user_filter, $pager_count) {
  global $user;

  $filters = _webform_confirm_email_get_submission_filters();

  require_once drupal_get_path('module', 'webform') . '/includes/webform.report.inc';
  $element = webform_results_submissions($node, $user_filter, $pager_count, $filters);

  $table = &$element['table'];
  unset($element['table']);

  $element['#theme']         = 'webform_confirm_email_results_submissions';
  $element['confirmed_form'] = drupal_get_form('webform_confirm_email_confirmed_filter_form');
  $element['table']          = &$table;

  return $element;
}

/**
 * Preprocess function for webform-confirm-email-results-submissions.tpl.php
 */
function template_preprocess_webform_confirm_email_results_submissions(&$vars) {
  $vars['node']           = $vars['element']['#node'];
  $vars['submissions']    = $vars['element']['#submissions'];
  $vars['confirmed_form'] = $vars['element']['confirmed_form'];
  $vars['table']          = $vars['element']['table'];
  $vars['total_count']    = $vars['element']['#total_count'];
  $vars['pager_count']    = $vars['element']['#pager_count'];
  $vars['is_submissions'] = (arg(2) == 'submissions')? 1 : 0;

  unset($vars['element']);
}

/**
 * Create a table containing all submitted values for a webform node.
 */
function webform_confirm_email_results_table($node, $pager_count = 0) {
  if (isset($_GET['results']) && is_numeric($_GET['results'])) {
    $pager_count = $_GET['results'];
  }
  $filters        = _webform_confirm_email_get_submission_filters();
  $header         = theme('webform_results_table_header', array('node' => $node));
  $submissions    = webform_get_submissions($filters, $header, $pager_count);
  $total_count    = webform_get_submission_count($node->nid, NULL, FALSE, $filters);
  $confirmed_form = drupal_get_form('webform_confirm_email_confirmed_filter_form');

  $output = theme(
    'webform_confirm_email_results_table',
    array(
      'confirmed_form' => $confirmed_form,
      'node'           => $node,
      'components'     => $node->webform['components'],
      'submissions'    => $submissions,
      'total_count'    => $total_count,
      'pager_count'    => $pager_count
    )
  );
  if ($pager_count) {
    $output .= theme('pager');
  }
  return $output;
}

function theme_webform_confirm_email_results_table($variables) {
  $output = drupal_render($variables['confirmed_form']);
  $output .= theme_webform_results_table($variables);

  return $output;
}
