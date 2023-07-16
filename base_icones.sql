INSERT INTO icons (`categorie`, `forme`, `extension`, `ihm_affichage`, `default_mode`, `default_color`) VALUES
('io',           'wago_750342',         'webp', 'static',        'default',    'black'  ),
('io',           'satellite',           'svg',  'static',        'default',    'black'  ),
('io',           'sms',                 'jpg',  'static',        'default',    'black'  ),
('divers',       'logo',                'png',  'static',        'default',    'black'  ),
('lieu',         'soisy',               'gif',  'static',        'default',    'black'  ),
('lieu',         '2d_soisy_niv00',      'png',  'static',        'default',    'black'  ),
('lieu',         '2d_soisy',            'gif',  'static',        'default',    'black'  ),
('chauffage',    'chaudiere_gaz',       'png',  'by_mode',       'off',        'black'  ),
('chauffage',    'radiateur',           'png',  'by_mode',       'chaud',      'black'  ),
('capteur',      'detecteur_mouvement', 'png',  'by_mode_color', 'off',        'white'  ),
('capteur',      'digicode',            'png',  'by_color',      'default',    'gray'   ),
('capteur',      'boite_aux_lettres',   'png',  'by_color',      'default',    'red'    ),
('bouton',       'auto_manu',           'svg',  'by_mode',       'auto',       'black'  ),
('ouvrant',      'fenetre',             'png',  'by_mode',       'ouverte',    'black'  ),
('ouvrant',      'porte_entree',        'png',  'by_mode',       'fermee',     'black'  ),
('ouvrant',      'porte_fenetre',       'png',  'by_mode',       'ouverte',    'black'  ),
('ouvrant',      'volet_roulant',       'png',  'by_mode',       'ouvert',     'black'  ),
('ouvrant',      '2d_porte',            'png',  'by_mode_color', 'ouverte',    'gray'   ),
('chauffage',    'soufflant',           'png',  'by_mode',       'off',        'black'  ),
('divers',       'cadena',              'png',  'by_mode',       'ouvert',     'black'  ),
('application',  'kodi',                'svg',  'static',        'default',    'black'  ),
('application',  'film',                'svg',  'static',        'default',    'black'  ),
('application',  'youtube_bebe_louis',  'png',  'static',        'default',    'black'  ),
('application',  'youtube_crocodile',   'png',  'static',        'default',    'black'  ),
('application',  'youtube_peppa_pig',   'png',  'static',        'default',    'black'  ),
('bouton',       'bouton_raz',          'png',  'by_color',      'default',    'blue'   ),
('bouton',       'arret_urgence',       'png',  'by_color',      'default',    'red'    ),
('bouton',       'bouton_panique',      'png',  'by_color',      'default',    'red'    ),
('panneau',      'panneau_stop',        'png',  'by_color',      'default',    'red'    ),
('panneau',      'panneau_au',          'png',  'by_color',      'default',    'red'    ),
('bouton',       'bouton_io',           'png',  'by_color',      'default',    'green'  ),
('electrique',   'ampoule',             'svg',  'by_mode',       'off',        'black'  ),
('electrique',   '2d_contacteur',       'svg',  'by_mode_color', 'ouvert',     'green'  ),
('electrique',   'pile',                'png',  'by_mode',       '100',        'black'  ),
('electrique',   'vmc',                 'png',  'by_color',      'default',    'green'  ),
('electrique',   'moteur',              'png',  'by_color',      'default',    'green'  ),
('electrique',   'moteur_2',            'png',  'by_color',      'default',    'green'  ),
('electrique',   'chauffe_eau',         'svg',  'by_color',      'default',    'blue'   ),
('outils',       'marteau_1',           'png',  'by_color',      'default',    'gray'   ),
('outils',       'clef_a_molette_1',    'png',  'by_color',      'default',    'gray'   ),
('outils',       'clef_a_molette_2',    'png',  'by_color',      'default',    'gray'   ),
('outils',       'clef_a_molette_3',    'png',  'by_color',      'default',    'gray'   ),
('voyant',       'voyant_moteur',       'png',  'by_color',      'default',    'orange' ),
('voyant',       'eclair',              'png',  'by_color',      'default',    'green'  ),
('voyant',       'voyant_local_marche', 'png',  'by_color',      'default',    'green'  ),
('voyant',       'voyant_local_arret',  'png',  'by_color',      'default',    'gray'   ),
('voyant',       'voyant_systeme',      'png',  'by_color',      'default',    'gray'   ),
('hydraulique',  '2d_vanne_auto',       'png',  'by_color',      'default',    'gray'   ),
('hydraulique',  '2d_vanne_manu',       'png',  'by_color',      'default',    'gray'   ),
('hydraulique',  'goutte_eau',          'png',  'by_color',      'default',    'blue'   ),
('voyant',       'clef',                'png',  'by_color',      'default',    'gray'   ),
('voyant',       'croix',               'png',  'by_color',      'default',    'red'    ),
('electrique',   '2d_ampoule',          'png',  'by_color',      'default',    'green'  ),
('divers',       'fleche',              'png',  'by_color',      'default',    'blue'   ),
('divers',       'check',               'png',  'by_mode' ,      'default',    'black'  ),
('chauffage',    'thermometre',         'png',  'by_mode_color', 'ok',         'white'  ),
('sonorisation', 'haut_parleur',        'svg',  'by_mode_color', 'actif',      'red'    ),
('bouton',       'bouton',              'none', 'complexe',      'enabled',    'blue'   ),
('divers',       'question',            'png',  'static',        'default',    'black'  ),
('divers',       'comment',             'none', 'complexe',      'annotation', 'black'  ),
('divers',       'encadre',             'none', 'complexe',      '1x1',        'white'  )
ON DUPLICATE KEY UPDATE categorie=VALUE(categorie), extension=VALUE(extension), ihm_affichage=VALUE(ihm_affichage),
                        default_mode=VALUE(default_mode), default_color=VALUE(default_color);
