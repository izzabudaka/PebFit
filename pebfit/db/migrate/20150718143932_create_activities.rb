class CreateActivities < ActiveRecord::Migration
  def change
    create_table :activities do |t|
      t.string :activity
      t.integer :lengthInSeconds
      
      t.timestamps null: false
    end
  end
end
