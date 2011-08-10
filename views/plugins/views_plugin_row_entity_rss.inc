<?php

/**
 * @file
 * Row style plugin for displaying the results as entities.
 */

/**
 * Plugin class for displaying Views results with entity_view.
 */
class views_plugin_row_entity_rss extends views_plugin_row {
  protected $entity_type, $skip_load, $entities = array();
	
  public function summary_title() {
  	return 'Entity RSS';
  }
	
	public function init(&$view, &$display, $options = NULL) {
		parent::init($view, $display, $options);

    $table_data = views_fetch_data($this->view->base_table);
    $this->entity_type = $table_data['table']['base']['entity type'];
    $this->skip_load = !empty($table_data['table']['base']['skip entity load']);

    $info = entity_get_info($this->entity_type);
    if (!empty($info['base table']) && $info['base table'] == $this->view->base_table) {
      $this->base_table = $info['base table'];
      $this->base_field = $info['entity keys']['id'];
    }
	}
	
	public function pre_render($results) {
		$this->entities = array();
    foreach ($results as $result) {
      if (!isset($result->entity)) {
        $id = entity_id($this->entity_type, $result);
        $this->entities[$id] = $result;
      }
      else {
        $id = (is_object($result->entity) ? entity_id($this->entity_type, $result->entity) : $result->entity);
        $this->entities[$id] = $result->entity;
      }
      // Force loading in case there is no object at all.
      if (!empty($id) && $id == $this->entities[$id]) {
        $this->skip_load = FALSE;
      }
    }
    if (!$this->skip_load) {
      $this->entities = entity_load($this->entity_type, array_keys($this->entities));
    }
	}
	
	public function render($row) {
		global $base_url;
		if (!isset($row->entity)) {
			$id = entity_id($this->entity_type, $row);
		}
		else {
      $id = (is_object($row->entity) ? entity_id($this->entity_type, $row->entity) : $row->entity);
    }
		$entity_info = entity_get_info($this->entity_type);
		if ($this->entities[$id]) {
			$entity = $this->entities[$id];
		}
		
		$entity->rss_namespaces = array('xmlns:dc' => 'http://purl.org/dc/elements/1.1/');
		$entity->rss_elements = array(
		  array('key' => 'pubData', 'value' => gmdate('r', $entity->created)),
		  array('key' => 'dc:creator', 'value' => $entity->name),
		  array('key' => 'guid', 'value' => $id . ' at ' . $base_url, 'attributes' => array('isPermaLink' => 'false')),
		);
		
		$content = entity_view($this->entity_type, array($id => $this->entities[$id]), 'rss');
		$item = new stdClass;
		$item->title = $entity->title;
		if (isset($entity_info['uri callback'])) {
			$link = $entity_info['uri callback']($entity);
			$item->link = url($link['path'], array('absolute' => TRUE));
		}
		$item->elements = $entity->rss_elements;
		$item->description = drupal_render($content);
		return theme($this->theme_functions(),
      array(
        'view' => $this->view,
        'options' => $this->options,
        'row' => $item
      ));
	}
}
